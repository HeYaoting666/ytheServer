#pragma once

#include "fd_event.h"

namespace ythe {

class WakeUpEvent : public FdEvent {
public:
    WakeUpEvent();
    
    ~WakeUpEvent() override = default;

public:
    void Wakeup();
};

}