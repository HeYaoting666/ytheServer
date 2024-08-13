#include "ip_net_addr.h"
#include "../log/logger.h"
#include <string.h>

bool checkValid(const std::string& addr, std::string& ip, uint16_t& iport) {
    size_t i = addr.find_first_of(':');
    if(i == std::string::npos) {
        return false;
    }

    ip = addr.substr(0, i);
    std::string port = addr.substr(i + 1, addr.size() - i - 1);
    if(ip.empty() || port.empty()) {
        return false;
    }

    iport = std::stoi(port);
    if (iport <= 0 || iport > 65536) {
        return false;
    }

    return true;
}

namespace ythe {

IPNetAddr::IPNetAddr(std::string& ip, uint16_t port): mIpStr(ip), mPort(port)
{
    memset(&mAddr, 0, sizeof(mAddr));
    mAddr.sin_family = AF_INET;
    mAddr.sin_port = htons(mPort);
    mAddr.sin_addr.s_addr = inet_addr(mIpStr.c_str());
}

IPNetAddr::IPNetAddr(const std::string& addr)
{
    size_t i = addr.find_first_of(':');
    if(!checkValid(addr, mIpStr, mPort)) {
        ERRORLOG("invalid ipv4 addr %s", addr.c_str())
        return;
    }

    memset(&mAddr, 0, sizeof(mAddr));
    mAddr.sin_family = AF_INET;
    mAddr.sin_port = htons(mPort);
    mAddr.sin_addr.s_addr = inet_addr(mIpStr.c_str());
}

IPNetAddr::IPNetAddr(sockaddr_in addr): mAddr(addr)
{
    mIpStr = std::string(inet_ntoa(mAddr.sin_addr));
    mPort  = ntohs(mAddr.sin_port);
}

bool IPNetAddr::IsValid()
{
    if (mIpStr.empty()) {
        return false;
    }
    if (inet_addr(mIpStr.c_str()) == INADDR_NONE) {
        return false;
    }
    if (mPort <= 0 || mPort > 65536) {
        return false;
    }
    return true;
}

}

