#include "wakeup_event.h"
#include "../log/logger.h"

namespace ythe {

WakeUpEvent::WakeUpEvent(): FdEvent()
{
    mFd = eventfd(0, EFD_NONBLOCK); // 创建wakeup_fd并设置为非阻塞
    if (mFd < 0) {
        ERRORLOG("failed to create event loop, eventfd create error, error info[%d]", errno)
        exit(0);
    }
    INFOLOG("create wakeup fd[%d]", mFd)

    auto cb = [this]() { // 定义回调函数
        char buf[8];
        while(read(mFd, buf, 8) != -1 && errno != EAGAIN) {}
        DEBUGLOG("read full bytes from wakeup fd[%d]", mFd)
    };
    SetFdEvent(IN_EVENT, cb); // 设置wakeup事件为读就绪同时传入对应的回调函数
}

void WakeUpEvent::Wakeup()
{
    char buf[8] = {'a'};
    auto ret = write(mFd, buf, 8);
    if(ret != 8)
        ERRORLOG("write to wakeup fd less than 8 bytes, fd[%d]", mFd)
    DEBUGLOG("success write %d bytes", ret)
}

}