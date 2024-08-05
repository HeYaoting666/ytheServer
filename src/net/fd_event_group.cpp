#include "fd_event_group.h"

namespace ythe {

void FdEventGroup::Init(int size)
{
    for (int i = 0; i < size; ++i) {
        mFdGroup.push_back(new FdEvent(i));
    }
}

FdEventGroup::~FdEventGroup()
{
    for (int i = 0; i < mFdGroup.size(); ++i) {
        if(mFdGroup[i] != nullptr) {
            delete mFdGroup[i];
            mFdGroup[i] = nullptr;
        }
    }
}

FdEvent *FdEventGroup::GetFdEvent(int fd)
{
    {
        std::lock_guard<std::mutex> lock(mMutex);
        if(fd < mFdGroup.size())
            return mFdGroup[fd];
        
        // 扩容
        int newSize = (int)(fd * 1.5);
        for(int i = mFdGroup.size(); i < newSize; ++i) {
            mFdGroup.push_back(new FdEvent(i));
        }
        return mFdGroup[fd];
    }
}

}