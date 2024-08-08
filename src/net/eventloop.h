#pragma once

#include <set>
#include <queue>
#include <functional>
#include <mutex>
#include <sys/epoll.h>
#include "wakeup_event.h"
#include "timer.h"

#define MAX_EVENT_NUMBER 100

namespace ythe {

class EventLoop {
private:
    pid_t         mThreadId;  // 标识本 event_loop 所属线程
    int           mEpollfd;   // epoll文件描述符
    std::set<int> mListenFds; // 管理已经被注册至epoll事件表的文件描述符

    WakeUpEvent*  mWakeUpEvent = nullptr;
    Timer*        mTimer       = nullptr;
    bool          mIsLooping   = false;

    std::mutex    mMutex;
    std::queue<std::function<void()>> mPendingTasks;  // eventloop 任务队列

public:
    EventLoop();

    ~EventLoop();

public:
    void Loop();

    void Stop(bool isWakeUp = true);

    void AddTimerEvent(const TimerEvent::sp& timeEvent);

    void AddFdEventToEpoll(FdEvent* fdEvent);

    void DeleteFdEventFromEpoll(FdEvent* fdEvent);

private:
    void addFdEventToEpoll(FdEvent* fdEvent);

    void deleteFdEventFromEpoll(FdEvent* fdEvent);

    void addTask(const std::function<void()>& cb, bool isWakeUp = false);

};

}