#pragma once
#include "eventloop.h"

namespace ythe {

class IOThread {
public:
    typedef std::shared_ptr<IOThread> sp;

private:
    pid_t                   mThreadId  = -1;
    std::thread             mThread;
    EventLoop*              mEventLoop = nullptr;

    std::mutex              mInitMutex;
    std::condition_variable mInitCond;
    bool                    mInitDone = false;

    std::mutex              mStartMutex;
    std::condition_variable mStartCond;
    bool                    mStart    = false;

public:
    IOThread();

    ~IOThread();

public:
    EventLoop* GetEventLoop() const { return mEventLoop; }

    void       Start();

    // 调用了这个函数的线程对象，一定要等这个线程对象的方法执行完毕后，这个join()函数才能得到返回。
    void       Join();

private:
    void       loop();
};

}