#pragma once

#include "fd_event.h"

namespace ythe {

class WakeUpEvent : public FdEvent {
public:
    WakeUpEvent();
    
    ~WakeUpEvent() { if(mFd > 0) close(mFd); }

public:
    void Wakeup();
};

}