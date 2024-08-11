#include "rpc_dispatcher.h"
#include "rpc_controller.h"
#include "rpc_closure.h"
#include "../log/logger.h"
#include "../tcp/connection.h"
#include "../entity/error_code.h"

namespace ythe {

void RpcDispatcher::Dispatch(const AbstractProtocol::sp& reqPb, const AbstractProtocol::sp& respPb, IPNetAddr::sp localAddr, IPNetAddr::sp peerAddr)
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

    // 取出 service 对象
    auto it = mServiceMap.find(serviceName); 
    if(it == mServiceMap.end()) {
        ERRORLOG("%s | service name[%s] not found", req->mMsgId.c_str(), serviceName.c_str())
        setTinyPBError(resp, ERROR_SERVICE_NOT_FOUND, "service not found");
        return;
    }
    auto service = it->second;

    // 根据 methodName 找到 service 对象的 method
    auto method = service->GetDescriptor()->FindMethodByName(methodName);
    if(method == nullptr) {
        ERRORLOG("%s | method name[%s] not found in service [%s]", req->mMsgId.c_str(), methodName.c_str(), serviceName.c_str())
        setTinyPBError(resp, ERROR_METHOD_NOT_FOUND, "method not found");
        return;
    }

    // 反序列化，将请求体 req 中的 pbData 反序列化
    auto reqMsg = service->GetRequestPrototype(method).New();
    auto respMsg = service->GetResponsePrototype(method).New();
    if(!reqMsg->ParseFromString(req->mPbData)) {
        ERRORLOG("%s | deserialize error", req->mMsgId.c_str())
        setTinyPBError(resp, ERROR_FAILED_DESERIALIZE, "deserialize error");
        delete reqMsg;
        return;
    }
    INFOLOG("%s | get rpc request[%s]", req->mMsgId.c_str(), reqMsg->ShortDebugString().c_str())

    // 初始化 rpcController
    auto rpcController = new RpcController();
    rpcController->SetLocalAddr(localAddr);
    rpcController->SetPeerAddr(peerAddr);
    rpcController->SetMsgId(req->mMsgId);

    // 初始化 rpcClosure
    auto rpcClosure = new RpcClosure([reqMsg, req, respMsg, resp]() {
        if( !respMsg->SerializeToString(&(resp->mPbData)) ) {
            ERRORLOG("%s | serialize error, origin message [%s]", req->mMsgId.c_str(), respMsg->ShortDebugString().c_str())
            setTinyPBError(resp, ERROR_FAILED_SERIALIZE, "serialize error");
        }
        else {
            resp->mErrCode = 0;
            resp->mErrInfo = "";
            INFOLOG("%s | dispatch success, request[%s], response[%s]", req->mMsgId.c_str(), reqMsg->ShortDebugString().c_str(), respMsg->ShortDebugString().c_str())
        }
    });

    // 调用 rpc 方法
    service->CallMethod(method, rpcController, reqMsg, respMsg, rpcClosure);
    delete rpcController;
    delete reqMsg;
    delete respMsg;
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