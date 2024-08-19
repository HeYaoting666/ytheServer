#include "src/config/config.h"
#include "src/log/logger.h"
#include "src/rpc/rpc_dispatcher.h"
#include "src/tcp/tcp_server.h"
#include "pb/compute.pb.h"

class ComputeService : public Compute {
public:
    void Add(google::protobuf::RpcController* controller, const Request* req, Response* resp, 
        google::protobuf::Closure* done) override {
            resp->set_z(req->x() + req->y());
            if(done) done->Run();
        }
};

int main() {
    auto configInstance = ythe::Config::GetInstance();
    configInstance->Init("/root/cpp/ytheServer/config_server.xml");

    auto logInstance = ythe::Logger::GetInstance();
    logInstance->Init();

    auto cumputeService = std::make_shared<ComputeService>();
    ythe::RpcDispatcher::GetInstance()->RegisterService(cumputeService);

    auto addr = std::make_shared<ythe::IPNetAddr>(configInstance->mServerIp, configInstance->mServerPort);
    ythe::TCPServer server(addr);
    server.Start();
    return 0;
}