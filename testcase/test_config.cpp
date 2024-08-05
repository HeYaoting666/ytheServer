#include "../src/config/config.h"
#include "../src/log/logger.h"
#include "../src/tcp/tcp_server.h"
#include <iostream>


int main() {
    auto configInstance = ythe::Config::GetInstance();
    configInstance->Init("/root/cpp/ytheRPC/server_config.xml");

    auto logInstance = ythe::Logger::GetInstance();
    logInstance->Init();

    // auto eventLoop = ythe::EventLoop::GetInstance();
    // eventLoop->Init();

    // auto timeEvent = std::make_shared<ythe::TimerEvent>(1000, true, [](){std::cout << "Server" << std::endl;});
    // eventLoop->AddTimerEvent(timeEvent);
    // eventLoop->Loop();//开始监听事件
    auto addr = std::make_shared<ythe::IPNetAddr>("127.0.0.1:9000");
    ythe::TCPServer server(addr);
    server.Start();
    return 0;
}