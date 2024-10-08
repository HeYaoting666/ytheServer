#pragma once
#include <google/protobuf/service.h>
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>

#include "src/tcp/ip_net_addr.h"
#include "src/coder/tinypb_protocol.h"

namespace ythe {

class RpcDispatcher {
public:
    typedef std::shared_ptr<google::protobuf::Service> serviceSp;

private:
    std::map<std::string, serviceSp> mServiceMap; // <服务名, 服务对象>

public:
    static RpcDispatcher* GetInstance() {
        static RpcDispatcher instance;
        return &instance;
    }

    RpcDispatcher(const RpcDispatcher&)  = delete;

    void operator=(const RpcDispatcher&) = delete;

private:
    RpcDispatcher() = default;
    
    ~RpcDispatcher() = default;

public:
    void Dispatch(const AbstractProtocol::sp& reqMessage, const AbstractProtocol::sp& respMessage, IPNetAddr::sp localAddr, IPNetAddr::sp peerAddr);

    // 注册prc服务对象
    void RegisterService(const serviceSp& service);

private:
    // 解析服务名称 "OrderServer.make_order" service_name = "OrderServer", method_name = "make_order"
    static bool parseServiceFullName(const std::string& fullName, std::string& serviceName, std::string& methodName);

    // 设置错误信息
    static void setTinyPBError(const std::shared_ptr<TinyPBProtocol>& msg, int32_t errCode, const std::string& errInfo);
};

}