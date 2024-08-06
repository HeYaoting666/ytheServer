#include "../src/config/config.h"
#include "../src/log/logger.h"
#include "../src/tcp/tcp_client.h"
#include <iostream>


int main() {
    auto configInstance = ythe::Config::GetInstance();
    configInstance->Init("/root/cpp/ytheRPC/server_config.xml");

    auto logInstance = ythe::Logger::GetInstance();
    logInstance->Init();

    auto addr = std::make_shared<ythe::IPNetAddr>("127.0.0.1:9000");
    ythe::TCPClient client(addr);
    client.TCPConnect();  
    client.Start();
    return 0;
}