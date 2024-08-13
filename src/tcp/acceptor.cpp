#include <cstring>
#include "acceptor.h"
#include "../log/logger.h"

namespace ythe {

TCPAcceptor::TCPAcceptor(const IPNetAddr::sp& localAddr): mLocalAddr(localAddr)
{
    if(!mLocalAddr->IsValid()) {
        ERRORLOG("invalid local addr %s", mLocalAddr->ToString().c_str())
        exit(0);
    }

    mListenfd = socket(mLocalAddr->GetFamily(), SOCK_STREAM, 0);
    if(mListenfd < 0) {
        ERRORLOG("invalid listenfd[%d]", mListenfd)
        exit(0);
    }

    // 设置listenfd属性，跳过 wait_time 快速重新使用ip：port
    int on = 1;
    if(setsockopt(mListenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) != 0) {
        ERRORLOG("setsockopt REUSEADDR error, errno=%d, error=%s", errno, strerror(errno))
        exit(0);
    }

    // bind() 将监听套接字与本地地址绑定
    if(bind(mListenfd, mLocalAddr->GetSockAddr(), mLocalAddr->GetSockLen()) != 0) {
        ERRORLOG("bind error, errno=%d, error=%s", errno, strerror(errno))
        exit(0);
    }

    // listen() 
    if(listen(mListenfd, 1000) != 0) {
        ERRORLOG("listen error, errno=%d, error=%s", errno, strerror(errno))
        exit(0);
    }
}

std::pair<int, IPNetAddr::sp> TCPAcceptor::TCPAccept()
{
    if(mLocalAddr->GetFamily() == AF_INET) {
        sockaddr_in clientSockAddr;
        socklen_t   clientSockAddrLen = sizeof(clientSockAddr);
        // 获取客户端连接套接字 clientFd 和客户端地址 clientAddr
        int clientFd = accept(mListenfd, reinterpret_cast<sockaddr*>(&clientSockAddr), &clientSockAddrLen);
        if(clientFd < 0) {
            ERRORLOG("accept error, errno=%d, error=%s", errno, strerror(errno))
            return {-1, nullptr};
        }

        IPNetAddr::sp clientAddr = std::make_shared<IPNetAddr>(clientSockAddr);
        INFOLOG("A client have been accepted successfully, client_fd [%d], client_addr [%s]", clientFd, clientAddr->ToString().c_str())
        return {clientFd, clientAddr};
    } else {
        // 其他协议实现...
        return {-1, nullptr};
    }
}

}