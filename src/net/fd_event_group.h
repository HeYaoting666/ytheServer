#pragma once
#include <vector>
#include "fd_event.h"

namespace ythe {

class FdEventGroup {
private:
    std::mutex             mMutex;
    std::vector<FdEvent *> mFdGroup;

public:
    static FdEventGroup* GetInstance() {
        static FdEventGroup instance;
        return &instance;
    }
    
    void Init(int size);

    FdEventGroup(const FdEventGroup&) = delete;

    void operator=(const FdEventGroup&) = delete; 

    ~FdEventGroup();

private:
    FdEventGroup() = default;

public:
    FdEvent* GetFdEvent(int fd);
    
};

}