#pragma once
#include "src/net/eventloop.h"
#include "src/abstract/abstract_protocol.h"
#include "src/abstract/abstract_coder.h"
#include "ip_net_addr.h"
#include "buffer.h"

namespace ythe {

enum TCPConnectionType {
    TCPConnectionByServer = 1,  // 作为服务端使用，代表跟对端客户端的连接
    TCPConnectionByClient = 2,  // 作为客户端使用，代表跟对端服务端的连接
};

enum TCPConnectionState {
    NotConnected = 1,
    Connected    = 2,
    HalfClosing  = 3,
    Closed       = 4,
};

class TCPConnection
{
public:
    typedef std::shared_ptr<TCPConnection> sp;

private:
    FdEvent*           mConnEvent;   // 连接事件描述符管理
    EventLoop*         mEventLoop;   // 持有该连接的IO线程的 event_loop 事件

    IPNetAddr::sp      mLocalAddr;   // 本地地址
    IPNetAddr::sp      mPeerAddr;    // 对端地址

    TCPBuffer::sp      mRecvBuffer;  // 接收缓冲区
    TCPBuffer::sp      mSendBuffer;  // 发送缓冲区

    TCPConnectionType  mType;        // 连接类型
    TCPConnectionState mState;       // 连接状态

    AbstractCoder*     mCoder;       // 编码解码器

public:
    TCPConnection(FdEvent* fdEvent, EventLoop* eventLoop, int bufferSize, 
        IPNetAddr::sp localAddr, IPNetAddr::sp peerAddr, TCPConnectionType type = TCPConnectionByServer);

    ~TCPConnection();

public:
    void               ListenReadEvent(bool setET = true);

    void               ListenWriteEvent(bool setET = true);

    void               CancelListenReadEvent(bool cancelET = false);

    void               CancelListenWriteEvent(bool cancelET = false);

    void               SetState(TCPConnectionState state) { mState = state; }

    void               SetConnectionType(TCPConnectionType type) { mType = type; }

    void               SetSendBuffer(const TCPBuffer::sp& sendBuffer) { mSendBuffer = sendBuffer; }

    TCPBuffer::sp      GetRecvBuffer()     const { return mRecvBuffer; }

    TCPConnectionState GetState()          const { return mState; }

    TCPConnectionType  GetConnectionType() const { return mType; }

    int                GetFd()             const { return mConnEvent->GetFd(); }

    IPNetAddr::sp      GetLocalAddr()      const { return mLocalAddr; }

    IPNetAddr::sp      GetPeerAddr()       const { return mPeerAddr; }

private:
    void onRead();      // 可读事件回调函数，读取客户发送的数据，组装为RPC请求

    void execute();     // 将 RPC 请求作为入参，执行业务逻辑得到 RPC 响应

    void onWrite();     // 可写事件回调函数，将RPC响应 out_buffer 发送给客户端

    void clear();       // 处理一些关闭连接后的清理动作
};


}