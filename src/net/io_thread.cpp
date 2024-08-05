#include "io_thread.h"
#include <iostream>

namespace ythe {

ythe::IOThread::IOThread()
{
    mThread = std::thread(&IOThread::loop, this);

    std::unique_lock<std::mutex> lock(mInitMutex);
    mInitCond.wait(lock, [this](){ return mInitDone; }); // 等待线程初始化完成
    DEBUGLOG("IOThread [%d] create success", mThreadId)
}

IOThread::~IOThread() {
    if (mEventLoop) {
        mEventLoop->Stop();
        delete mEventLoop;
        mEventLoop = nullptr;
    }
}

void IOThread::Start()
{
    DEBUGLOG("Now invoke IOThread [%d]", mThreadId)
    {
        std::lock_guard<std::mutex> lock(mStartMutex);
        mStart = true;
    }
    mStartCond.notify_one();
}

void IOThread::Join()
{
    if (mThread.joinable()) {
        mThread.join();
    }
}

void IOThread::loop()
{
    mThreadId = GetThreadId();
    mEventLoop = new EventLoop();
    std::cout << mEventLoop << std::endl;
    {
        std::lock_guard<std::mutex> lock(mInitMutex);
        mInitDone = true;
    }
    mInitCond.notify_one();

    // 让IO 线程等待，直到主线程主动启动
    DEBUGLOG("IOThread [%d] wait start semaphore", mThreadId)
    {
        std::unique_lock<std::mutex> lock(mStartMutex);
        mStartCond.wait(lock, [this](){ return mStart; });
    }

    DEBUGLOG("IOThread [%d] start loop ", mThreadId)
    mEventLoop->Loop();
    DEBUGLOG("IOThread [%d] end loop ", mThreadId)
}

}