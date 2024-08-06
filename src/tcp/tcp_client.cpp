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
        INFOLOG("client connect error, errorno: %d, error msg: %s", mConnectErrorCode, mConnectErrorInfo)
        exit(0);
    }
}

void TCPClient::onConnect()
{
    // EINPROGRESS 表示连接建立过程仍在进行中，此时需要通过后续的操作
    // 如 select() 或 epoll_wait() 来检查套接字状态，以确定连接是否已成功建立或失败。
    int ret = connect(mFdEvent->GetFd(), mPeerAddr->GetSockAddr(), mPeerAddr->GetSockLen());
    if (ret == -1 && errno != EINPROGRESS) {
        mConnectErrorCode = ERROR_FAILED_CONNECT;
        mConnectErrorInfo = "connect error, sys error = " + std::string(strerror(errno));
        return;
    }
    if(ret == -1 && errno == EINPROGRESS) {
        onNonBlockingConnect();
        return;
    }
    if (ret == 0) {
        onConnectSuccess();
        return;
    }
}

void TCPClient::onConnectSuccess()
{
    INFOLOG("connect [%s] success", mPeerAddr->ToString().c_str())
    mConn->SetState(Connected);
}

void TCPClient::onNonBlockingConnect()
{
    mFdEvent->SetFdEvent(OUT_EVENT, std::bind(&TCPClient::handleNonBlockingConnect, this));
    mEventLoop->AddFdEventToEpoll(mFdEvent);
}

void TCPClient::handleNonBlockingConnect()
{
    int ret = connect(mFdEvent->GetFd(), mPeerAddr->GetSockAddr(), mPeerAddr->GetSockLen());
    if ((ret < 0 && errno == EISCONN) || (ret == 0)) {
        onConnectSuccess();
    } else {
        if (errno == ECONNREFUSED) {
            mConnectErrorCode = ERROR_PEER_CLOSED;
            mConnectErrorInfo = "connect refused, sys error = " + std::string(strerror(errno));
        } else {
            mConnectErrorCode = ERROR_FAILED_CONNECT;
            mConnectErrorInfo = "connect unknown error, sys error = " + std::string(strerror(errno));
        }
    }
    // 连接完后需要在“epoll事件表”中去掉可写事件的监听，不然会一直触发
    mFdEvent->CancelFdEvent(OUT_EVENT);
    mEventLoop->AddFdEventToEpoll(mFdEvent);
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