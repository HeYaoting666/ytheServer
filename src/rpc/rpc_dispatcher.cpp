#include "rpc_dispatcher.h"
#include "../log/logger.h"
#include "../entity/error_code.h"

namespace ythe {

void RpcDispatcher::Dispatch(const AbstractProtocol::sp& reqPb, const AbstractProtocol::sp& respPb, TCPConnection* conn)
{
    auto req = std::dynamic_pointer_cast<TinyPBProtocol>(reqPb);
    auto resp = std::dynamic_pointer_cast<TinyPBProtocol>(respPb);
    resp->mMsgId = req->mMsgId;
    resp->mMethodName = req->mMethodName;

    // 获取请求消息方法名并解析出 serviceName 和 methodName
    std::string methodFullName = req->mMethodName;
    std::string serviceName;
    std::string methodName;
    if(!parseServiceFullName(methodFullName, serviceName, methodName)) {
        setTinyPBError(resp, ERROR_PARSE_SERVICE_NAME, "parse service name error");
        return;
    }
}

void RpcDispatcher::RegisterService(const serviceSp& service)
{
    std::string serviceName = service->GetDescriptor()->full_name();
    mServiceMap[serviceName] = service;
}

bool RpcDispatcher::parseServiceFullName(const std::string& fullName, std::string& serviceName, std::string& methodName)
{
    if(fullName.empty()) {
        ERRORLOG("%s", "full name empty")
        return false;
    }

    size_t i = fullName.find_first_of('.');
    if(i == std::string::npos) {
        ERRORLOG("not find . in full name [%s]", fullName.c_str())
        return false;
    }
    serviceName = fullName.substr(0, i);
    methodName = fullName.substr(i + 1, fullName.length() - 1 - i);
    INFOLOG("parse service_name[%s] and method_name[%s] from full name [%s]", serviceName.c_str(), methodName.c_str(), fullName.c_str())

    return true;
}

void RpcDispatcher::setTinyPBError(const std::shared_ptr<TinyPBProtocol>& msg, int32_t errCode, const std::string& errInfo)
{
    msg->mErrCode = errCode;
    msg->mErrInfo = errInfo;
    msg->mErrInfoLen = errInfo.length();
}

}