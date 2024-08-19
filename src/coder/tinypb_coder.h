#pragma once
#include "src/abstract/abstract_coder.h"

namespace ythe {

class TinyPBCoder: public AbstractCoder{
public:
    // 将 message 对象转化为字节流，写入到 in_buffer，编码
    void Encode(std::vector<AbstractProtocol::sp>& messages, TCPBuffer::sp buffer) override;

    // 将 buffer 里面的字节流转换为 message 对象，解码
    void Decode(TCPBuffer::sp buffer, std::vector<AbstractProtocol::sp>& messages) override;
};

}