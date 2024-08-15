#include "tcp_client.h"
#include "../log/logger.h"
#include "../config/config.h"
#include "../entity/error_code.h"

namespace ythe {

TCPClient::TCPClient(const IPNetAddr::sp &peerAddr): mPeerAddr(peerAddr)
{
    mConnFd = socket(mPeerAddr->GetFamily(), SOCK_STREAM, 0);
    if(mConnFd < 0) {
        ERRORLOG("%s", "client error, failed to create fd")
        return;
    }
    mFdEvent = new FdEvent(mConnFd);
    mFdEvent->SetNonBlock();

    mEventLoop = new EventLoop();

    mBufferSize = Config::GetInstance()->mClientBufferSize;

    initLocalAddr(); // 初始化本地地址
    mConn = std::make_shared<TCPConnection>(
        mFdEvent, mEventLoop, mBufferSize, mLocalAddr, mPeerAddr, TCPConnectionByClient); 
}

TCPClient::~TCPClient()
{
    if(mConnFd > 0)
        close(mConnFd);

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
        printf("client connect error, errorno: %d, error msg: %s\n", mConnectErrorCode, mConnectErrorInfo.c_str());
        exit(1);
    }
}

void TCPClient::TCPDisConnect()
{   if(mConn->GetState() == NotConnected)
        return;
    
    mConn->SetState(NotConnected);
    if(mEventLoop) 
        mEventLoop->Stop();
    if(mConnFd > 0) 
        close(mConnFd);
}

void TCPClient::OneCall(const TCPBuffer::sp &sendData, TCPBuffer::sp &recvData)
{
    if(mConn->GetState() != Connected) {
        ERRORLOG("fd[%d] no connect, one call error", mConnFd)
        return;
    }
    mConn->SetSendBuffer(sendData);
    mConn->ListenWriteEvent();
    StartEventLoop();
    recvData = mConn->GetRecvBuffer();
}

void TCPClient::onConnect()
{
    int ret = connect(mConnFd, mPeerAddr->GetSockAddr(), mPeerAddr->GetSockLen());
    if (ret == -1 && errno == EINPROGRESS) {
        ret = connect(mConnFd, mPeerAddr->GetSockAddr(), mPeerAddr->GetSockLen());
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

    int ret = getsockname(mConnFd, reinterpret_cast<sockaddr*>(&localSocketAddr), &len);
    if(ret != 0) {
        ERRORLOG("init local addr error, getsockname error, [errno=%d, error=%s]", errno, strerror(errno));
        return;
    }
    mLocalAddr = std::make_shared<IPNetAddr>(localSocketAddr);
}

}