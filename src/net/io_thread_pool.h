#pragma once
#include <vector>
#include "io_thread.h"

namespace ythe {

class IOThreadPool {
private:
    int mSize  = 3;
    int mIndex = 0;

    std::vector<IOThread *> mIOThreadGroup;

public:
    static IOThreadPool* GetInstance() {
        static IOThreadPool instance;
        return &instance;
    }
    void Init(int size);

    IOThreadPool(const IOThreadPool&) = delete;

    void operator=(const IOThreadPool&) = delete;

    ~IOThreadPool();

private:
    IOThreadPool() = default;

public:
    void      Start();       // 开启每个 IOThread 的 EventLoop事件
    void      Join();        // 回收线程组当中每个线程
    IOThread* GetIOThread(); // 获取线程组当中的线程句柄
};

}