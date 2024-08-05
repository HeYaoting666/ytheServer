#include "fd_event.h"

namespace ythe {

void FdEvent::SetFdEvent(TriggerEvent event, const std::function<void()>& cb, const std::function<void()>& errCb)
{
    if(event == IN_EVENT) {
        mEpollEvent.events |= EPOLLIN;
        mReadCallBack = cb;
    } else if (event == OUT_EVENT) {
        mEpollEvent.events |= EPOLLOUT;
        mReadCallBack = cb;
    }

    if(errCb == nullptr)
        mErrorCallBack = nullptr;
    else
        mErrorCallBack = errCb;

    // 将本文件描述符管理指针传给 mEvent.data.ptr 用于后续加入epoll内核事件表中
    mEpollEvent.data.ptr = this;
}

void FdEvent::SetEpollET()
{
    if(mEpollEvent.events & EPOLLET)
        return;
    mEpollEvent.events |= EPOLLET;
}

void FdEvent::CancelFdEvent(TriggerEvent event)
{
    if(event == IN_EVENT)
        mEpollEvent.events &= (~EPOLLIN);
    else if(event == OUT_EVENT)
        mEpollEvent.events &= (~EPOLLOUT);
}

void FdEvent::CancelEpollET()
{
    if(mEpollEvent.events & EPOLLET)
        mEpollEvent.events &= (~EPOLLET);
}

void FdEvent::SetNonBlock()
{
    int flag = fcntl(mFd, F_GETFL);
    if (flag & O_NONBLOCK)
        return;
    fcntl(mFd, F_SETFL, flag | O_NONBLOCK);
}

std::function<void()> FdEvent::GetCallBack(TriggerEvent event) const
{
    switch (event) {
        case IN_EVENT:
            return mReadCallBack;
        case OUT_EVENT:
            return mWriteCallBack;
        case ERROR_EVENT:
            return mErrorCallBack;
        default:
            return nullptr;
    }
}

}