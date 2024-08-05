#pragma once

#include "../log/logger.h"
#include "../abstract/net_addr.h"

namespace ythe {

class IPNetAddr: public NetAddr {
private:
    std::string    mIpStr;
    uint16_t       mPort;
    sockaddr_in    mAddr;

public:
    IPNetAddr(std::string& ip, uint16_t port);    // 以 "ip" + "port" 的形式初始化

    explicit IPNetAddr(const std::string& addr); // 以 "ip:port" 的形式初始化

    explicit IPNetAddr(sockaddr_in addr);        // 以 "sockaddr_in" 的形式初始化

    ~IPNetAddr() override = default;

public:
    sockaddr*   GetSockAddr() override   { return reinterpret_cast<sockaddr *>(&mAddr); }

    socklen_t   GetSockLen()  override   { return sizeof(mAddr); }

    int         GetFamily()   override   { return mAddr.sin_family; }

    std::string ToString()    override   { return mIpStr + ":" + std::to_string(mPort); }

    bool        IsValid()     override;
};

}