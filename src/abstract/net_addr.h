#pragma once

#include <arpa/inet.h>
#include <memory>

namespace ythe {

class NetAddr {
public:
    typedef std::shared_ptr<NetAddr> sp;

public:
    virtual ~NetAddr() = default; //抽象基类虚函数需要定义为虚函数

public:
    virtual sockaddr*   GetSockAddr() = 0;

    virtual socklen_t   GetSockLen()  = 0;

    virtual int         GetFamily()   = 0;

    virtual std::string ToString()    = 0;

    virtual bool        IsValid()     = 0;
};

}