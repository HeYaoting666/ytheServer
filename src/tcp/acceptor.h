#pragma once
#include <unistd.h>
#include "ip_net_addr.h"

namespace ythe {

/***********************************************************
 *  TCPAcceptor： tcp连接监听套接字管理
 *  负责初始化tcp连接监听套接字，处理相关tcp连接accept
 ***********************************************************/
class TCPAcceptor {
public:
    typedef std::shared_ptr<TCPAcceptor> sp;

private:
    IPNetAddr::sp   mLocalAddr;   // 服务端监听的地址
    int             mListenfd;    // 监听套接字

public:
    explicit TCPAcceptor(const IPNetAddr::sp& localAddr); // socket(), bind(), listen()

    ~TCPAcceptor() { if(mListenfd > 0) close(mListenfd); }
public:
    std::pair<int, IPNetAddr::sp> TCPAccept(); // 处理tcp连接，返回<连接套接字, 客户端地址>

    int           GetListenFd()  const { return mListenfd; }

    IPNetAddr::sp GetLocalAddr() const { return mLocalAddr; }
};

}