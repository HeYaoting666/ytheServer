#pragma once
#include <functional>
#include <map>
#include <fcntl.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/timerfd.h>
#include "../log/logger.h"

namespace ythe {

/****************************************
 *    文件描述符事件管理 FdEvent
 *    管理 fd, epoll_event 和对应的回调函数
 *****************************************/

enum TriggerEvent {
    IN_EVENT    =   EPOLLIN,
    OUT_EVENT   =   EPOLLOUT,
    ERROR_EVENT =   EPOLLERR,
};

class FdEvent {
protected:
    int           mFd = -1;
    epoll_event   mEpollEvent;

    std::function<void()> mReadCallBack;
    std::function<void()> mWriteCallBack;
    std::function<void()> mErrorCallBack;

public:
    FdEvent() = default;

    explicit FdEvent(int fd): mFd(fd) { }

    virtual ~FdEvent() { if(mFd > 0) close(mFd); };

public:
    // 设置监听事件和相应的回调函数，用于后续epoll事件注册
    void                  SetFdEvent(TriggerEvent event, const std::function<void()>& cb, const std::function<void()>& errCb = nullptr);

    // 设置监听事件为ET模式
    void                  SetEpollET();

    // 取消监听事件
    void                  CancelFdEvent(TriggerEvent event_type);

    // 取消监听事件ET模式
    void                  CancelEpollET();

    void                  SetNonBlock();

    int                   GetFd() const { return mFd; }

    epoll_event           GetEpollEvent() const { return mEpollEvent; }

    std::function<void()> GetCallBack(TriggerEvent event) const;

};

}