#include <arpa/inet.h>
#include <cstring>
#include "tinypb_coder.h"
#include "tinypb_protocol.h"
#include "../utils/utils.h"

namespace ythe {

void TinyPBCoder::Encode(std::vector<AbstractProtocol::sp>& messages, TCPBuffer::sp buffer)
{
    for (auto msg: messages) {
        auto message = std::dynamic_pointer_cast<TinyPBProtocol>(msg);

        uint32_t packageLen = 2 + 20 + message->mMsgId.length() + message->mMethodName.length() + 
            message->mErrInfo.length() + message->mPbData.length();
        char* buf = reinterpret_cast<char*>(malloc(packageLen));
        char* tmp = buf;

        // 加入开始符标志
        *tmp = TinyPBProtocol::PB_START;
        ++tmp;

        // 加入 packageLen
        uint32_t packageLenNet = htonl(packageLen); // 转换为网络字节序
        memcpy(tmp, &packageLenNet, sizeof(packageLenNet));
        tmp += sizeof(packageLenNet);

        // 加入 msgIdLen
        uint32_t msgIdLen = message->mMsgId.length();
        uint32_t msgIdLenNet = htonl(msgIdLen);
        memcpy(tmp, &msgIdLenNet, sizeof(msgIdLenNet));
        tmp += sizeof(msgIdLenNet);
        // 加入 msgId
        if(!message->mMsgId.empty()) {
            memcpy(tmp, &(message->mMsgId[0]), msgIdLen);
            tmp += msgIdLen;
        }

        // 加入 methodNameLen
        uint32_t methodNameLen = message->mMethodName.length();
        uint32_t methodNameLenNet = htonl(methodNameLen);
        memcpy(tmp, &methodNameLenNet, sizeof(methodNameLenNet));
        tmp += sizeof(methodNameLenNet);
        // 加入 methodName
        if(!message->mMethodName.empty()) {
            memcpy(tmp, &(message->mMethodName[0]), methodNameLen);
            tmp += methodNameLen;
        }

        // 加入 err_code
        uint32_t errCodeNet = htonl(message->mErrCode);
        memcpy(tmp, &errCodeNet, sizeof(errCodeNet));
        tmp += sizeof(errCodeNet);
        // 加入 mErrInfoLen
        uint32_t errInfoLen = message->mErrInfo.length();
        uint32_t errInfoLenNet = htonl(errInfoLen);
        memcpy(tmp, &errInfoLenNet, sizeof(errInfoLenNet));
        tmp += sizeof(errInfoLenNet);
        // 加入 methodName
        if(!message->mErrInfo.empty()) {
            memcpy(tmp, &(message->mErrInfo[0]), errInfoLen);
            tmp += errInfoLen;
        }

        // 加入 pbData
        if(!message->mPbData.empty()) {
            memcpy(tmp, &(message->mPbData[0]), message->mPbData.length());
            tmp += message->mPbData.length();
        }

         // 加入结束符标志
        *tmp = TinyPBProtocol::PB_END;

        message->mPackageLen = packageLen;
        message->mMsgIdLen = msgIdLen;
        message->mMethodNameLen = methodNameLen;
        message->mErrInfoLen = errInfoLen;
        message->mParseSuccess = true;
        DEBUGLOG("encode message[%s] success", message->mMsgId.c_str());

        // 将编码后的字节流写入至 in_buffer 中
        if(buf != nullptr && packageLen != 0)
            buffer->WriteToBuffer(buf, packageLen);

        // 释放分配的内存
        if(buf != nullptr)
            free(buf);
    }
}

void TinyPBCoder::Decode(TCPBuffer::sp buffer, std::vector<AbstractProtocol::sp>& messages)
{
    while(true) {
        char* tmp = buffer->Data();
        uint32_t startIndex = buffer->ReadIndex();
        uint32_t endIndex = -1;
        uint32_t packageLen = 0;
        bool parseSuccess = false;

        // 遍历 buffer，找到 PB_START，找到之后解析出整包的长度，然后得到结束符的位置，判断是否为 PB_END
        int i;
        for(i = startIndex; i < buffer->WriteIndex(); ++i) {
            if(tmp[i] == TinyPBProtocol::PB_START && i < buffer->WriteIndex()) {
                 // 读下去四个字节获取整包长度，由于是网络字节序，需要转为主机字节序
                packageLen = GetInt32FromNetByte(&tmp[i + 1]);

                // 结束符的索引
                int j = i + packageLen - 1;
                if(tmp[j] == TinyPBProtocol::PB_END) {
                    startIndex = i;
                    endIndex = j;
                    parseSuccess = true;
                    break;
                }
            }
        }
        if(i >= buffer->WriteIndex()) {
            DEBUGLOG("%s", "decode end, read all buffer data")
            break;
        }

        if(parseSuccess == true) {
            std::shared_ptr<TinyPBProtocol> message = std::make_shared<TinyPBProtocol>();
            message->mPackageLen= packageLen;

            // 解析 msg_id_len
            uint32_t msgIdLenIndex = startIndex + sizeof(TinyPBProtocol::PB_START) + sizeof(message->mPackageLen);
            if (msgIdLenIndex >= endIndex) {
                message->mParseSuccess = false;
                ERRORLOG("parse error, msg_id_len_index[%d] >= end_index[%d]", msgIdLenIndex, endIndex)
                continue;
            }
            message->mMsgIdLen = GetInt32FromNetByte(&tmp[msgIdLenIndex]);
            DEBUGLOG("parse msg_id_len=%d", message->mMsgIdLen)

            // 解析 msg_id
            uint32_t msgIdIndex = msgIdLenIndex + sizeof(message->mMsgIdLen);
            message->mMsgId = std::string(&tmp[msgIdIndex], message->mMsgIdLen);
            DEBUGLOG("parse msg_id=%s", message->mMsgId.c_str())

            // 解析 method_name_len
            uint32_t methodNameLenIndex = msgIdIndex +  message->mMsgIdLen;
            if (methodNameLenIndex >= endIndex) {
                message->mParseSuccess = false;
                ERRORLOG("parse error, method_name_len_index[%d] >= end_index[%d]", methodNameLenIndex, endIndex)
                continue;
            }
            message->mMethodNameLen = GetInt32FromNetByte(&tmp[methodNameLenIndex]);
            DEBUGLOG("parse method_name_len=%d", message->mMethodNameLen)

            // 解析 method_name
            uint32_t methodNameIndex = methodNameLenIndex + sizeof(message->mMethodNameLen);
            message->mMethodName = std::string(&tmp[methodNameIndex], message->mMethodNameLen);
            DEBUGLOG("parse method_name=%s", message->mMethodName)

            // 解析 err_code
            uint32_t errCodeIndex = methodNameIndex + message->mMethodNameLen;
            if (errCodeIndex >= endIndex) {
                message->mParseSuccess = false;
                ERRORLOG("parse error, err_code_index[%d] >= end_index[%d]", errCodeIndex, endIndex)
                continue;
            }
            message->mErrCode = GetInt32FromNetByte(&tmp[errCodeIndex]);

            // 解析 error_info_len
            uint32_t errorInfoLenIndex = errCodeIndex + sizeof(message->mErrCode);
            if (errorInfoLenIndex >= endIndex) {
                message->mParseSuccess = false;
                ERRORLOG("parse error, error_info_len_index[%d] >= end_index[%d]", errorInfoLenIndex, endIndex)
                continue;
            }
            message->mErrInfoLen = GetInt32FromNetByte(&tmp[errorInfoLenIndex]);

            // 解析 error_info
            uint32_t errInfoIndex = errorInfoLenIndex + sizeof(message->mErrInfoLen);
            message->mErrInfo = std::string(&tmp[errInfoIndex], message->mErrInfoLen);
            DEBUGLOG("parse error_info=%s", message->mErrInfo.c_str())

            // 解析 pb_data
            uint32_t pdDataIndex = errInfoIndex + message->mErrInfoLen;
            uint32_t pbDataLen = message->mPackageLen - message->mMsgIdLen - message->mMethodNameLen - message->mErrInfoLen - 2 - 20;
            message->mPbData = std::string(&tmp[pdDataIndex], pbDataLen);

            message->mParseSuccess = true;

            // 将解析结果放入out_message, 并重新调整 buffer 中 readIndex 位置
            messages.push_back(message);
            buffer->MoveReadIndex(endIndex - startIndex + 1);
        }
    }
}

}