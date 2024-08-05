#include "connection.h"
#include "../net/fd_event.h"

namespace ythe {

TCPConnection::TCPConnection(int fd, EventLoop* eventLoop, int bufferSize, 
    IPNetAddr::sp localAddr, IPNetAddr::sp peerAddr, TCPConnectionType type)
    : mEventLoop(eventLoop), mLocalAddr(localAddr), mPeerAddr(peerAddr), mType(type)
{
    // 初始化连接状态
    mState = NotConnected;

    // 初始化接收发送缓冲区
    mRecvBuffer = std::make_shared<TCPBuffer>(bufferSize);
    mSendBuffer = std::make_shared<TCPBuffer>(bufferSize);

    // 初始化 fdEvent 事件
    mConnEvent = new FdEvent(fd);
    mConnEvent->SetNonBlock();
    if(mType == TCPConnectionByServer)
        ListenReadEvent();
}

TCPConnection::~TCPConnection()
{
    if(mConnEvent) {
        delete mConnEvent;
        mConnEvent = nullptr;
    }
}

void TCPConnection::ListenReadEvent(bool isET)
{
    mConnEvent->SetFdEvent(TriggerEvent::IN_EVENT, std::bind(&TCPConnection::onRead, this));
    if(isET)
        mConnEvent->SetEpollET();
    mEventLoop->AddFdEventToEpoll(mConnEvent);
}

void TCPConnection::ListenWriteEvent(bool isET)
{
    mConnEvent->SetFdEvent(TriggerEvent::OUT_EVENT, std::bind(&TCPConnection::onWrite, this));
    if(isET)
        mConnEvent->SetEpollET();
    mEventLoop->AddFdEventToEpoll(mConnEvent);
}

// socket 内核缓冲区 --> recvBuffer 应用层缓冲区
void TCPConnection::onRead()
{
    if (mState != Connected) {
        ERRORLOG("onRead error, client has already disconnected, addr[%s], clientfd[%d]", mPeerAddr->ToString().c_str(), mConnEvent->GetFd())
        return;
    }

    bool isClose = false;
    while(true) {
        if(mRecvBuffer->WriteAble() == 0) {
            mRecvBuffer->ResizeBuffer(2 * mRecvBuffer->Size());
        }

        int readCount = mRecvBuffer->WriteAble();
        int writeIndex = mRecvBuffer->WriteIndex();
        int ret = read(mConnEvent->GetFd(), &(*mRecvBuffer)[writeIndex], readCount);
        DEBUGLOG("success read %d bytes from addr[%s], client fd[%d]", ret, mPeerAddr->ToString().c_str(), mConnEvent->GetFd())
        if(ret == 0) {  // 客户端断开连接
            isClose = true;
            break;
        }
        if (ret == -1) {
            if(errno == EAGAIN || errno == EWOULDBLOCK) // 全部读取完成
                break;
            else {
                ERRORLOG("onRead Error from addr[%s], client fd[%d]", mPeerAddr->ToString().c_str(), mConnEvent->GetFd())
                return;
            }
        }
        mRecvBuffer->MoveWriteIndex(ret);
    }
    if(isClose) {
        INFOLOG("peer closed, peer addr [%s], clientfd [%d]", mPeerAddr->ToString().c_str(), mConnEvent->GetFd())
        clear();
        return;
    }

    execute();
}

void TCPConnection::onWrite()
{

}

void TCPConnection::execute()
{
    
}

void TCPConnection::clear()
{
    if(mState == Closed)
        return;

    mEventLoop->DeleteFdEventFromEpoll(mConnEvent);
    mState = Closed;
}

}