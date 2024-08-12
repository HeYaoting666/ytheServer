#include "../src/config/config.h"
#include "../src/log/logger.h"
#include "../pb/compute.pb.h"
#include "../src/rpc/rpc_channel.h"
#include "../src/rpc/rpc_closure.h"

int main() {
    auto configInstance = ythe::Config::GetInstance();
    configInstance->Init("/root/cpp/ythe_server/config.xml");

    auto logInstance = ythe::Logger::GetInstance();
    logInstance->Init();
    
    // 定义rpc channel
    auto addr = std::make_shared<ythe::IPNetAddr>(configInstance->mConnectIp, configInstance->mConnectPort);
    auto channel = std::make_shared<ythe::RpcChannel>(addr);
    channel->ConnectToServer();

    // 定义请求消息 reqPb 和相应消息体 respPb
    auto reqPb = std::make_shared<Request>();
    auto respPb = std::make_shared<Response>();
    reqPb->set_x(1);
    reqPb->set_y(1);
    
    // 定义rpc controller
    auto controller = std::make_shared<ythe::RpcController>();

    // 定义rpc closure
    auto closure = std::make_shared<ythe::RpcClosure>([reqPb, respPb, controller]{
        if(controller->GetErrorCode() == 0) {
            INFOLOG("call rpc success, request[%s], response[%s]", reqPb->ShortDebugString().c_str(), respPb->ShortDebugString().c_str())
            // 执行业务逻辑
        } else {
            ERRORLOG("call rpc failed, request[%s], error code[%d], error info[%s]",
                     reqPb->ShortDebugString().c_str(),
                     controller->GetErrorCode(),
                     controller->GetErrorInfo().c_str())
        }
    });
    
    // 定义rpc stub 执行rpc调用
    Compute_Stub computeStub(channel.get());
    computeStub.Add(controller.get(), reqPb.get(), respPb.get(), closure.get());
    sleep(2);
    computeStub.Add(controller.get(), reqPb.get(), respPb.get(), closure.get());
    computeStub.Add(controller.get(), reqPb.get(), respPb.get(), closure.get());
    channel->DisConnectFomServer();

    return 0;
}