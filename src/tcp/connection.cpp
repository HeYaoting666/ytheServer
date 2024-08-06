#include "connection.h"
#include "../net/fd_event.h"

namespace ythe {

TCPConnection::TCPConnection(FdEvent* fdEvent, EventLoop* eventLoop, int bufferSize, 
    IPNetAddr::sp localAddr, IPNetAddr::sp peerAddr, TCPConnectionType type)
    : mConnEvent(fdEvent), mEventLoop(eventLoop), mLocalAddr(localAddr), mPeerAddr(peerAddr), mType(type)
{
    // 初始化连接状态
    mState = NotConnected;

    // 初始化接收发送缓冲区
    mRecvBuffer = std::make_shared<TCPBuffer>(bufferSize);
    mSendBuffer = std::make_shared<TCPBuffer>(bufferSize);

    if(mType == TCPConnectionByServer)
        ListenReadEvent();
}

void TCPConnection::ListenReadEvent(bool setET)
{
    mConnEvent->SetFdEvent(IN_EVENT, std::bind(&TCPConnection::onRead, this));
    if(setET)
        mConnEvent->SetEpollET();
    mEventLoop->AddFdEventToEpoll(mConnEvent);
}

void TCPConnection::ListenWriteEvent(bool setET)
{
    mConnEvent->SetFdEvent(OUT_EVENT, std::bind(&TCPConnection::onWrite, this));
    if(setET)
        mConnEvent->SetEpollET();
    mEventLoop->AddFdEventToEpoll(mConnEvent);
}

void TCPConnection::CancelListenReadEvent(bool cancelET)
{
    mConnEvent->CancelFdEvent(IN_EVENT);
    if(cancelET)
        mConnEvent->CancelEpollET();
    mEventLoop->AddFdEventToEpoll(mConnEvent);
}

void TCPConnection::CancelListenWriteEvent(bool cancelET)
{
    mConnEvent->CancelFdEvent(OUT_EVENT);
    if(cancelET)
        mConnEvent->CancelEpollET();
    mEventLoop->AddFdEventToEpoll(mConnEvent);
}

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

        int writeSize = mRecvBuffer->WriteAble();
        int writeStart = mRecvBuffer->WriteIndex();
        // fd socket 内核缓冲区 --> recvBuffer 应用层缓冲区
        int ret = read(mConnEvent->GetFd(), mRecvBuffer->Data() + writeStart, writeSize);
        DEBUGLOG("success read %d bytes from addr[%s], client fd[%d]", ret, mPeerAddr->ToString().c_str(), mConnEvent->GetFd())
        if (ret == 0) {  // 客户端断开连接
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
    if (mState != Connected) {
        ERRORLOG("onWrite error, client has already disconnected, addr[%s], clientfd[%d]", 
            mPeerAddr->ToString().c_str(), mConnEvent->GetFd())
        return;
    }

    bool isWriteAll = false;
    while(true) {
        if (mSendBuffer->ReadAble() == 0) {
            DEBUGLOG("%s", "no data need to send")
            isWriteAll = true;
            break;
        }

        int readSize = mSendBuffer->ReadAble();
        int readStart = mSendBuffer->ReadIndex();
        // sendBuffer 应用层缓冲区 --> fd socket 内核缓冲区
        int ret = write(mConnEvent->GetFd(), mRecvBuffer->Data() + readStart, readSize) ;
        if(ret == -1 && errno == EAGAIN) {
            // 发送缓冲区已满，不能再发送了。
            // 等待下次 fd 可写的时候再次发送数据即可
            ERRORLOG("%s", "write data error, errno==EAGAIN and rt == -1")
            break;
        }
        DEBUGLOG("success write %d bytes to addr[%s], client fd[%d]", ret,  mPeerAddr->ToString().c_str(), mConnEvent->GetFd())
        mRecvBuffer->MoveReadIndex(ret);
    }
    if(isWriteAll) {
        CancelListenWriteEvent();
    }
}

void TCPConnection::execute()
{
    INFOLOG("%s", "execute")
}

void TCPConnection::clear()
{
    if(mState == Closed)
        return;

    mEventLoop->DeleteFdEventFromEpoll(mConnEvent);
    mState = Closed;
}

}