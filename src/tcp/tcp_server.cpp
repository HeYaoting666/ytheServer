#include "tcp_server.h"
#include <iostream>

namespace ythe {

TCPServer::TCPServer(const IPNetAddr::sp& localAddrr)
{
    // 初始化 TCPAcceptor
    mAcceptor = std::make_shared<TCPAcceptor>(localAddrr);
    mLocalAddr = mAcceptor->GetLocalAddr();

    // 初始化 主线程 EventLoop
    mEventLoop = new EventLoop();
    std::cout << mEventLoop << std::endl;

    // 初始化 监听套接字事件 mListenEvent 不使用ET模式, 加入到epoll事件表中
    mListenEvent = new FdEvent(mAcceptor->GetListenFd());
    mListenEvent->SetFdEvent(TriggerEvent::IN_EVENT, std::bind(&TCPServer::onAccept, this));
    mEventLoop->AddFdEventToEpoll(mListenEvent);

    // 初始化 从线程 IOThreadGroup
    mIOThreadPool = IOThreadPool::GetInstance();
    mIOThreadPool->Init();

    // 初始化 定时器事件 m_clear_client_timer_event
    mTimerEvent = std::make_shared<TimerEvent>(5000, true, std::bind(&TCPServer::onClearClientTimerFunc, this));
    mEventLoop->AddTimerEvent(mTimerEvent);

    INFOLOG("rocket TCPServer listen success on [%s]", mLocalAddr->ToString().c_str())
}

TCPServer::~TCPServer()
{
    if(mIOThreadPool) {
        mIOThreadPool->Join();
        delete mIOThreadPool;
        mIOThreadPool = nullptr;
    }

    if(mEventLoop) {
        mEventLoop->Stop();
        delete mEventLoop;
        mEventLoop = nullptr;
    }

    if (mListenEvent) {
        delete mListenEvent;
        mListenEvent = nullptr;
    }
}

void TCPServer::Start()
{
    mIOThreadPool->Start();
    mEventLoop->Loop();
}

void TCPServer::onAccept()
{
    auto [clientFd, clientAddr] = mAcceptor->TCPAccept();
    auto threadHander = mIOThreadPool->GetIOThread();

    // 将 clientFd 和 clientAddr 绑定到从属线程的eventloop中
    auto conn = std::make_shared<TCPConnection>(
        clientFd, threadHander->GetEventLoop(), 128, clientAddr, mLocalAddr);
    mConnClients.insert(conn);
    INFOLOG("TCPServer success get client, addr[%s], fd[%d]", clientAddr->ToString().c_str(), clientFd)
}

void TCPServer::onClearClientTimerFunc()
{
    for (auto it = mConnClients.begin(); it != mConnClients.end(); ) {
        if ((*it) != nullptr && (*it).use_count() > 0 && (*it)->GetState() == Closed) {
            DEBUGLOG("TCPConnection [fd:%d] will delete, state=%d", (*it)->GetFd(), (*it)->GetState());
            it = mConnClients.erase(it);
        } else {
            ++it;
        }
    }
}

}