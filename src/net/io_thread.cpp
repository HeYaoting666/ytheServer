#include "io_thread.h"
#include "src/log/logger.h"

namespace ythe {

ythe::IOThread::IOThread()
{
    mThread = std::thread(&IOThread::loop, this);
    {
        std::unique_lock<std::mutex> lock(mInitMutex);
        mInitCond.wait(lock, [this](){ return mInitDone; }); // 等待线程初始化完成
    }
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
    {
        std::lock_guard<std::mutex> lock(mInitMutex);
        mInitDone = true;
    }
    mInitCond.notify_one();

    // 让IO 线程等待，直到主线程主动启动
    {
        std::unique_lock<std::mutex> lock(mStartMutex);
        mStartCond.wait(lock, [this](){ return mStart; });
    }

    DEBUGLOG("IOThread [%d] start loop ", mThreadId)
    mEventLoop->Loop();
}

}