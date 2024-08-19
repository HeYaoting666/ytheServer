#pragma once
#include "src/abstract/abstract_protocol.h"

namespace ythe {

struct TinyPBProtocol: public AbstractProtocol {
public:
    static char   PB_START;            // 开始符 0x02 (1B)
    static char   PB_END;              // 结束符 0x03 (1B)

public:
    uint32_t      mPackageLen = 0;     // 整包长度 (4B)
    
    uint32_t      mMsgIdLen = 0;       // msg_id 的长度(4B)，msg_id 继承父类

    uint32_t      mMethodNameLen = 0;  // 方法名长度 (4B)
    std::string   mMethodName;         // 方法名

    uint32_t      mErrCode = 0;        // 错误码 (4B)
    uint32_t      mErrInfoLen = 0;     // 错误信息长度 (4B)
    std::string   mErrInfo;            // 错误信息

    std::string   mPbData;             // 序列化数据
public:
    TinyPBProtocol() = default;

    ~TinyPBProtocol() = default;
};

}