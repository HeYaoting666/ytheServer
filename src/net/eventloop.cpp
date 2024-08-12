#include "eventloop.h"

namespace ythe {

EventLoop::EventLoop()
{
    mThreadId = GetThreadId();
    mEpollfd = epoll_create1(EPOLL_CLOEXEC); // 现代epoll创建方式
    if(mEpollfd == -1) {
        ERRORLOG("failed to create event loop, epoll_create error, error info[%d]", errno)
        exit(0);
    }

    mWakeUpEvent = new WakeUpEvent();
    addFdEventToEpoll(mWakeUpEvent);

    mTimer = new Timer();
    addFdEventToEpoll(mTimer);

    INFOLOG("success create event loop in thread[%d]", mThreadId)
}

EventLoop::~EventLoop()
{
    close(mEpollfd);
    if(mWakeUpEvent) {
        delete mWakeUpEvent;
        mWakeUpEvent = nullptr;
    }
    if(mTimer) {
        delete mTimer;
        mTimer = nullptr;
    }
}

void EventLoop::Loop()
{
    mIsLooping = true;
    epoll_event events[MAX_EVENT_NUMBER];

    // 执行队列中的任务
    while(mIsLooping) {
        std::queue< std::function<void()> > tmpTasks;
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mPendingTasks.swap(tmpTasks);
        }

        while(!tmpTasks.empty()) {
            auto cb = std::move(tmpTasks.front());
            tmpTasks.pop();
            if(cb) cb();
        }

        // DEBUGLOG("%s", "now begin to epoll_wait")
        int numEvents = epoll_wait(mEpollfd, events, MAX_EVENT_NUMBER, -1);
        // DEBUGLOG("now end epoll_wait, num_events = %d", numEvents)
        if (numEvents < 0 && errno != EINTR) {
            ERRORLOG("epoll_wait error, errno=%d, error=%s", errno, strerror(errno))
            continue;
        }
        for(int i = 0; i < numEvents; ++i) {
            epoll_event triggerEvent = events[i];
            auto fdEvent = static_cast<FdEvent *>(triggerEvent.data.ptr);
            if(!fdEvent) {
                ERRORLOG("%s", "fd_event = NULL, continue")
                continue;
            }
            if(triggerEvent.events & EPOLLIN) {
                DEBUGLOG("fd[%d] trigger EPOLLIN event", fdEvent->GetFd())
                addTask(fdEvent->GetCallBack(IN_EVENT));
            }
            if(triggerEvent.events & EPOLLOUT) {
                DEBUGLOG("fd[%d] trigger EPOLLOUT event", fdEvent->GetFd())
                addTask(fdEvent->GetCallBack(OUT_EVENT));
            }
            if(triggerEvent.events & EPOLLERR) {
                DEBUGLOG("fd[%d] trigger EPOLLERROR event", fdEvent->GetFd())
                if(fdEvent->GetCallBack(ERROR_EVENT) != nullptr) {
                    DEBUGLOG("fd[%d] add error callback", fdEvent->GetFd())
                    addTask(fdEvent->GetCallBack(ERROR_EVENT));
                }
                deleteFdEventFromEpoll(fdEvent); // 删除出错的套接字
            }
        }
    }
}

void EventLoop::Stop()
{
    if(mIsLooping == false)
        return;
        
    mIsLooping = false;
    DEBUGLOG("%s", "Stop EventLoop")
    mWakeUpEvent->Wakeup();
}

void EventLoop::AddTimerEvent(const TimerEvent::sp& timeEvent)
{
    mTimer->AddTimerEvent(timeEvent);
    INFOLOG("%s", "addTimerEvent success")
}

void EventLoop::AddFdEventToEpoll(FdEvent* fdEvent)
{
    // 当前操作的线程与event_loop所属线程不同，将任务加入从属event_loop的任务队列中
    // 通过wakeup唤醒从属线程的event_loop，执行任务队列中的任务
    if(GetThreadId() == mThreadId) {
        addFdEventToEpoll(fdEvent);
    } else {
        auto cb = [this, fdEvent]() { addFdEventToEpoll(fdEvent); };
        addTask(cb, true);
    }
}

void EventLoop::addFdEventToEpoll(FdEvent* fdEvent)
{
    int op = EPOLL_CTL_ADD;
    if(mListenFds.find(fdEvent->GetFd()) != mListenFds.end())
        op = EPOLL_CTL_MOD;
    auto events = fdEvent->GetEpollEvent();
    int rt = epoll_ctl(mEpollfd, op, fdEvent->GetFd(), &events);
    if (rt == -1) {
        ERRORLOG("failed epoll_ctl when add fd[%d], errno=%d, error=%s", fdEvent->GetFd(), errno, strerror(errno));
        exit(0);
    }
    mListenFds.insert(fdEvent->GetFd());

    INFOLOG("add event success fd[%d], events: %s", fdEvent->GetFd(), EpollEventsToString(fdEvent->GetEpollEvent()).c_str())
}

void EventLoop::DeleteFdEventFromEpoll(FdEvent* fdEvent)
{
    // 当前操作的线程与event_loop所属线程不同，将任务加入从属event_loop的任务队列中
    // 通过wakeup唤醒从属线程的event_loop，执行任务队列中的任务
    if(GetThreadId() == mThreadId) {
        deleteFdEventFromEpoll(fdEvent);
    } else {
        auto cb = [this, fdEvent]() { deleteFdEventFromEpoll(fdEvent); };
        addTask(cb, true);
    }
}

void EventLoop::deleteFdEventFromEpoll(FdEvent* fdEvent)
{
    int op = EPOLL_CTL_DEL;
    if(mListenFds.find(fdEvent->GetFd()) == mListenFds.end())
        return;
    
    int rt = epoll_ctl(mEpollfd, op, fdEvent->GetFd(), nullptr);
    if (rt == -1) {
        ERRORLOG("failed epoll_ctl when delete fd[%d], errno=%d, error=%s", fdEvent->GetFd(), errno, strerror(errno));
        exit(0);
    }

    mListenFds.erase(fdEvent->GetFd());
    DEBUGLOG("delete event success, fd[%d]", fdEvent->GetFd())
}

void EventLoop::addTask(const std::function<void()> &cb, bool isWakeUp)
{
    {
        std::lock_guard<std::mutex> lock(mMutex);
        mPendingTasks.push(cb);
    }
    if(isWakeUp) {
        INFOLOG("%s", "WAKE UP")
        mWakeUpEvent->Wakeup();
    }
}

}