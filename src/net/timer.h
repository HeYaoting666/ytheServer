#pragma once
#include <mutex>
#include <map>
#include "timer_event.h"
#include "fd_event.h"

namespace ythe {

/************************************
 *  Timer 用于管理TimerEvent
 ************************************/
class Timer: public FdEvent {
private:
    std::multimap<int64_t, TimerEvent::sp> mPendingEvents; // 根据事务的 arrive_time 进行排序
    std::mutex mMutex;

public:
    Timer();

    ~Timer() { if(mFd > 0) close(mFd); }
    
public:
    void AddTimerEvent(const TimerEvent::sp& timeEvent);

private:
    void onTimer(); // 定时器回调函数，并删除超时的定时事件

    void resetTimerFd();

};

}