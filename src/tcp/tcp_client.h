#pragma once
#include <memory>
#include "../net/fd_event.h"
#include "../net/eventloop.h"
#include "ip_net_addr.h"
#include "connection.h"

namespace ythe {

class TCPClient {
public:
    typedef std::shared_ptr<TCPClient> sp;

private:
    FdEvent*          mFdEvent   = nullptr;
    EventLoop*        mEventLoop = nullptr;
    TCPConnection::sp mConn;
    IPNetAddr::sp     mPeerAddr;   // 对端地址
    IPNetAddr::sp     mLocalAddr;  // 本地地址

    int               mConnectErrorCode = 0;
    std::string       mConnectErrorInfo = "";

public:
    explicit TCPClient(const IPNetAddr::sp&  peerAddr);

    ~TCPClient();

public:
    // 异步进行 connect，即非阻塞connect
    // 如果 connect 完成，done 会被执行
    void TCPConnect();

    void Start() { mEventLoop->Loop(); }

private:;
    // 处理连接逻辑
    void onConnect();

    // 处理连接成功的逻辑
    void onConnectSuccess();

    // 处理非阻塞连接的逻辑
    void onNonBlockingConnect();
    
    // 非阻塞连接回调函数
    void handleNonBlockingConnect();

    void initLocalAddr();
};

}