#include "../src/config/config.h"
#include "../src/log/logger.h"
#include "../pb/compute.pb.h"
#include "../src/rpc/rpc_channel.h"
#include "../src/rpc/rpc_closure.h"


int main() {
    auto configInstance = ythe::Config::GetInstance();
    configInstance->Init("/root/cpp/ytheRPC/server_config.xml");

    auto logInstance = ythe::Logger::GetInstance();
    logInstance->Init();

    auto addr = std::make_shared<ythe::IPNetAddr>("127.0.0.1:9000");
    
    // 定义rpc channel
    auto channel = std::make_shared<ythe::RpcChannel>(addr);
    auto reqPb = std::make_shared<Request>();
    auto respPb = std::make_shared<Response>();
    reqPb->set_x(1);
    reqPb->set_y(1);
    
    // 定义rpc controller
    auto controller = std::make_shared<ythe::RpcController>();
    controller->SetMsgId("99998888");
    controller->SetTimeout(5000);

    // 定义rpc closure
    auto closure = std::make_shared<ythe::RpcClosure>([reqPb, respPb, controller]{
        if(controller->GetErrorCode() == 0) {
            INFOLOG("call rpc success, request[%s], response[%s]", reqPb->ShortDebugString().c_str(), respPb->ShortDebugString().c_str())
        }
    });
    
    // 定义rpc stub
    Compute_Stub computeStub(channel.get());
    computeStub.Add(controller.get(), reqPb.get(), respPb.get(), closure.get());

    return 0;
}