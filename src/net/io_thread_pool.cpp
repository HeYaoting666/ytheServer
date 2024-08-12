#include "io_thread_pool.h"

namespace ythe {

void IOThreadPool::Init(int size)
{
    mSize = size;
    mIOThreadGroup.resize(mSize);
    for(int i = 0; i < mSize; ++i) {
        mIOThreadGroup[i] = new IOThread();
    }
}

IOThreadPool::~IOThreadPool()
{
    for(auto thread: mIOThreadGroup) {
        if(thread)
            delete thread;
        thread = nullptr;
    }
    mIOThreadGroup.clear();
}

void IOThreadPool::Start()
{
    for(auto thread: mIOThreadGroup) {
        thread->Start();
    }
}

void IOThreadPool::Join()
{
    for(auto thread: mIOThreadGroup) {
        thread->Join();
    }
}

IOThread* IOThreadPool::GetIOThread()
{   auto tmp = mIOThreadGroup[mIndex];
    mIndex = (mIndex + 1) % mSize;
    return tmp;
}

}