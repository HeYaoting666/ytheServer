#pragma once
#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>

#include "../tcp/ip_net_addr.h"
#include "../tcp/tcp_client.h"
#include "rpc_controller.h"

namespace ythe {

class RpcChannel: public google::protobuf::RpcChannel {
private:
    AbstractCoder*  mCoder;    
    TCPClient::sp   mClient;

public:
    RpcChannel(const IPNetAddr::sp& peerAddr);

    ~RpcChannel() override;

public:
    void ConnectToServer() { if(mClient) mClient->TCPConnect(); }

    void DisConnectFomServer() { if(mClient) mClient->TCPDisConnect(); }

    void CallMethod(const google::protobuf::MethodDescriptor* method,
                   google::protobuf::RpcController* rpcController,
                   const google::protobuf::Message* reqPb,
                   google::protobuf::Message* respPb,
                   google::protobuf::Closure* done ) override;
};

}