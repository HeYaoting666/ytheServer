#pragma once
#include "acceptor.h"
#include "connection.h"
#include "../net/eventloop.h"
#include "../net/io_thread_pool.h"

namespace ythe {

class TCPServer {
private:
    FdEvent*                    mListenEvent;   // 监听套接字事件描述符管理
    EventLoop*                  mEventLoop;     // 主线程事件循环，负责监听连接套接字
    IPNetAddr::sp               mLocalAddr;

    IOThreadPool*               mIOThreadPool;
    std::set<TCPConnection::sp> mConnClients;

    TCPAcceptor::sp             mAcceptor;
    TimerEvent::sp              mTimerEvent;

public:
    explicit TCPServer(const IPNetAddr::sp& localAddrr);

    ~TCPServer();

public:
    void Start();    // 启动从线程事件循环和主线程事件循环

private:
    void onAccept(); // 连接套接字回调函数，负责处理客户连接

    void onClearClientTimerFunc(); // 定时器回调函数，清除 closed 的连接
};

}