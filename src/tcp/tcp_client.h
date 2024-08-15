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
    int               mConnFd     = -1;
    FdEvent*          mFdEvent    = nullptr;
    EventLoop*        mEventLoop  = nullptr;
    int               mBufferSize = 128;
    TCPConnection::sp mConn;
    IPNetAddr::sp     mPeerAddr;   // 对端地址
    IPNetAddr::sp     mLocalAddr;  // 本地地址

    int               mConnectErrorCode = 0;
    std::string       mConnectErrorInfo = "";

public:
    explicit TCPClient(const IPNetAddr::sp&  peerAddr);

    ~TCPClient();

public:
    void TCPConnect();

    void TCPDisConnect();

    void OneCall(const TCPBuffer::sp& sendData, TCPBuffer::sp& recvData);

    void AddTimerEvent(const TimerEvent::sp& timerEvent) {
        mEventLoop->AddTimerEvent(timerEvent);
    }

    void StopEventLoop() { mEventLoop->Stop(); }

    void StartEventLoop() { mEventLoop->Loop(); }

    IPNetAddr::sp GetLocalAddr() const { return mLocalAddr; }

    IPNetAddr::sp GetPeerAddr()  const { return mPeerAddr; }

private:;
    // 处理连接逻辑
    void onConnect();

    void initLocalAddr();
};

}