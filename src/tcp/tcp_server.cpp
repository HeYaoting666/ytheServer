#include "tcp_server.h"
#include "src/log/logger.h"
#include "src/config/config.h"

namespace ythe {

TCPServer::TCPServer(const IPNetAddr::sp& localAddrr)
{   
    mBuffersize = Config::GetInstance()->mSeverBufferSize;
    mIOThreadPoolNum = Config::GetInstance()->mIOThreadNums;

    // 初始化 TCPAcceptor
    mAcceptor = std::make_shared<TCPAcceptor>(localAddrr);
    mLocalAddr = mAcceptor->GetLocalAddr();

    // 初始化 监听套接字事件 mListenEvent 不使用ET模式, 加入到epoll事件表中
    mListenEvent = new FdEvent(mAcceptor->GetListenFd());
    mListenEvent->SetFdEvent(TriggerEvent::IN_EVENT, std::bind(&TCPServer::onAccept, this));

    // 初始化 从线程 IOThreadGroup
    mIOThreadPool = IOThreadPool::GetInstance();
    mIOThreadPool->Init(mIOThreadPoolNum);

    // 初始化 定时器事件(日志写入，清除断开连接)
    mTimerEvent = std::make_shared<TimerEvent>(10000, true, std::bind(&TCPServer::onClearClientTimerFunc, this));

    // 初始化 主线程 EventLoop
    mEventLoop = new EventLoop();
    mEventLoop->AddFdEventToEpoll(mListenEvent);
    mEventLoop->AddTimerEvent(mTimerEvent);
    mEventLoop->AddTimerEvent(Logger::GetInstance()->GetTimeEvent());

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
    FdEvent* clientFdEvent = new FdEvent(clientFd);
    clientFdEvent->SetNonBlock(); // ET模式设置非阻塞

    auto conn = std::make_shared<TCPConnection>(
        clientFdEvent, threadHander->GetEventLoop(), mBuffersize, mLocalAddr, clientAddr);
    conn->SetState(Connected);

    mConnClients.insert(conn);
    mClientFdEventsMap[clientFd] = clientFdEvent;
    INFOLOG("TCPServer success get client, addr[%s], fd[%d]", clientAddr->ToString().c_str(), clientFd)
}

void TCPServer::onClearClientTimerFunc()
{
    for (auto it = mConnClients.begin(); it != mConnClients.end(); ) {
        auto conn = *it;
        if (conn != nullptr && conn.use_count() > 0 && conn->GetState() == Closed) {
            DEBUGLOG("delete  TCPConnection [fd:%d], state=%d", conn->GetFd(), conn->GetState())
            close(conn->GetFd());
            delete mClientFdEventsMap[conn->GetFd()];
            mClientFdEventsMap.erase(conn->GetFd());

            it = mConnClients.erase(it);  // 从连接集合中删除并更新迭代器，避免迭代器失效
        } else {
            ++it;
        }
    }
}

}
