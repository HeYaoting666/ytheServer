#include "tcp_client.h"
#include "../entity/error_code.h"

namespace ythe {

TCPClient::TCPClient(const IPNetAddr::sp &peerAddr): mPeerAddr(peerAddr)
{
    mEventLoop = new EventLoop();
    int fd = socket(mPeerAddr->GetFamily(), SOCK_STREAM, 0);
    if(fd < 0) {
        ERRORLOG("%s", "client error, failed to create fd")
        return;
    }
    mFdEvent = new FdEvent(fd);
    mFdEvent->SetNonBlock();

    initLocalAddr(); // 初始化本地地址
    mConn = std::make_shared<TCPConnection>(
        mFdEvent, mEventLoop, 128, mLocalAddr, mPeerAddr, TCPConnectionByClient);
        
}

TCPClient::~TCPClient()
{
    if(mEventLoop) {
        mEventLoop->Stop();
        delete mEventLoop;
        mEventLoop = nullptr;
    }

    if (mFdEvent) {
        delete mFdEvent;
        mFdEvent = nullptr;
    }
}

void TCPClient::TCPConnect()
{
    onConnect();
    if(mConnectErrorCode != 0) {
        INFOLOG("client connect error, errorno: %d, error msg: %s", mConnectErrorCode, mConnectErrorInfo.c_str())
        exit(1);
    }
}

void TCPClient::SendData(const TCPBuffer::sp& sendData)
{
    if(mConn->GetState() != Connected) {
        ERRORLOG("fd[%d] no connect, sendData error", mConn->GetFd())
        return;
    }
    mConn->SetSendBuffer(sendData);
    mConn->ListenWriteEvent();
    mEventLoop->Loop();
}

void TCPClient::RecvData(TCPBuffer::sp& recvData)
{
    if(mConn->GetState() != Connected) {
        ERRORLOG("fd[%d] no connect, sendData error", mConn->GetFd())
        return;
    }
    recvData = mConn->GetRecvBuffer();
}

void TCPClient::onConnect()
{
    int ret = connect(mFdEvent->GetFd(), mPeerAddr->GetSockAddr(), mPeerAddr->GetSockLen());
    if (ret == -1 && errno == EINPROGRESS) {
        ret = connect(mFdEvent->GetFd(), mPeerAddr->GetSockAddr(), mPeerAddr->GetSockLen());
        if (ret == -1 && errno != EINPROGRESS) {
            mConnectErrorCode = ERROR_FAILED_CONNECT;
            mConnectErrorInfo = "connect unknown error, sys error = " + std::string(strerror(errno));
            return;
        }
        INFOLOG("connect [%s] success", mPeerAddr->ToString().c_str())
        mConn->SetState(Connected);
        return;
    }
    if (ret == -1 && errno != EINPROGRESS) {
        if (errno == ECONNREFUSED) {
            mConnectErrorCode = ERROR_PEER_CLOSED;
            mConnectErrorInfo = "connect refused, sys error = " + std::string(strerror(errno));
        } else {
            mConnectErrorCode = ERROR_FAILED_CONNECT;
            mConnectErrorInfo = "connect unknown error, sys error = " + std::string(strerror(errno));
        }
        return;
    }

    INFOLOG("connect [%s] success", mPeerAddr->ToString().c_str())
    mConn->SetState(Connected);
    return;
}

void TCPClient::initLocalAddr()
{
    sockaddr_in localSocketAddr{};
    socklen_t len = sizeof(localSocketAddr);

    int ret = getsockname(mFdEvent->GetFd(), reinterpret_cast<sockaddr*>(&localSocketAddr), &len);
    if(ret != 0) {
        ERRORLOG("init local addr error, getsockname error, [errno=%d, error=%s]", errno, strerror(errno));
        return;
    }
    mLocalAddr = std::make_shared<IPNetAddr>(localSocketAddr);
}

}