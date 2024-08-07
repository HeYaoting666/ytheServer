#pragma once

#include <vector>
#include "abstract_protocol.h"
#include "../tcp/buffer.h"

namespace ythe {

class AbstractCoder {
public:
    // 将 message 对象转化为字节流，写入到 buffer
    virtual void Encode(std::vector<AbstractProtocol::sp>& reqMessages, TCPBuffer::sp buffer) = 0;

    // 将 buffer 里面的字节流转换为 message 对象
    virtual void Decode(TCPBuffer::sp buffer, std::vector<AbstractProtocol::sp>& respMessages) = 0;

    virtual ~AbstractCoder() = default;
};

}