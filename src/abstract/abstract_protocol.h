#pragma once
#include <memory>

namespace ythe {

struct AbstractProtocol : public std::enable_shared_from_this<AbstractProtocol> {
public:
    typedef std::shared_ptr<AbstractProtocol> sp;

    virtual ~AbstractProtocol() = default;

public:
    std::string mMsgId;     // 请求号，唯一标识一个请求或者响应
};

}