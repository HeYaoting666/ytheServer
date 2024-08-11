#include "../src/config/config.h"
#include "../src/log/logger.h"
#include "../src/tcp/tcp_client.h"
#include "../src/coder/tinypb_coder.h"
#include "../src/coder/tinypb_protocol.h"


int main() {
    // auto configInstance = ythe::Config::GetInstance();
    // configInstance->Init("/root/cpp/ytheRPC/server_config.xml");

    // auto logInstance = ythe::Logger::GetInstance();
    // logInstance->Init();

    // auto addr = std::make_shared<ythe::IPNetAddr>("127.0.0.1:9000");
    // ythe::TCPClient client(addr);

    // auto msg = std::make_shared<ythe::TinyPBProtocol>();
    // msg->mMsgId = "123456";
    // msg->mMethodName = "Add";
    // msg->mPbData = "1, 2";
    // std::vector<ythe::AbstractProtocol::sp> vec;
    // vec.push_back(msg);
    // auto coder = std::make_shared<ythe::TinyPBCoder>();
    // auto buffer = std::make_shared<ythe::TCPBuffer>(128);
    // coder->Encode(vec, buffer);
    // // coder->Decode(buffer, vec);


    // client.TCPConnect();  
    // client.SendData(buffer);
    
    return 0;
}