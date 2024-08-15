#include "timer.h"
#include "../log/logger.h"
#include "../utils/utils.h"
namespace ythe {

Timer::Timer(): FdEvent()
{
    mFd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);

    // 设置 mFd 监听事件和回调函数
    SetFdEvent(IN_EVENT, std::bind(&Timer::onTimer, this));
}

void Timer::AddTimerEvent(const TimerEvent::sp& timeEvent)
{
    if (!timeEvent) return;
                
    bool needReset = false;
    {
        std::lock_guard<std::mutex> lock(std::mutex);
        if(mPendingEvents.empty()) {
            needReset = true;
        } else {
            auto it = mPendingEvents.begin();
            // 若插入任务的指定时间比所有任务的指定时间都要早，则需要重新修改定时任务指定时间
            if((*it).second->GetArriveTime() > timeEvent->GetArriveTime())
                needReset = true;
        }
        mPendingEvents.emplace(timeEvent->GetArriveTime(), timeEvent);
    }

    if(needReset) resetTimerFd();
}

void Timer::onTimer()
{
    // 处理缓冲区数据
    char buf[128];
    while (true) {
        if((read(mFd, buf, 128) == -1) && errno == EAGAIN)
            break;
    }

    // 从定时队列中提取出超时的timeEvent
    auto nowTime = GetNowMs();
    std::vector<TimerEvent::sp> outDateEvent; // 存储超时的TimerEvent
    {
        std::lock_guard<std::mutex> lock(std::mutex);
        auto it = mPendingEvents.begin();
        for(;it != mPendingEvents.end(); ++it) {
            auto arriveTime = it->first;
            auto timeEvent = it->second;
            if(arriveTime <= nowTime) {
                if(timeEvent->IsCancel())
                    continue;
                outDateEvent.push_back(timeEvent);
            }
            else {
                break;
            }
        }
        mPendingEvents.erase(mPendingEvents.begin(), it);
    }

    // 执行定时任务, 并把需要重复执行的 TimerEvent 再次添加至队列中
    for(const auto& timeEvent : outDateEvent) {
        // 执行定时任务
        auto timeCb = timeEvent->GetCallBack();
        if(timeCb) timeCb();

        // 把需要重复执行的 TimerEvent 再次添加至队列中
        if(timeEvent->IsRepeated()) {
            timeEvent->ResetArriveTime();   // 调整 arrive_time
            AddTimerEvent(timeEvent);      // 添加回队列中，重新设置timer_fd
        }
    }
}

void Timer::resetTimerFd()
{
    int64_t earliestArriveTime;
    // 取出定时器中最早时间
    {
        std::lock_guard<std::mutex> lock(std::mutex);
        if(mPendingEvents.empty()) {
            return;
        }
        auto it = mPendingEvents.begin();
        earliestArriveTime = it->second->GetArriveTime();
    }

    int64_t interval = 0;
    int64_t nowTime = GetNowMs();
    if(earliestArriveTime > nowTime)
        interval = earliestArriveTime - nowTime;
    else
        interval = 100;

    // 给 mFd 设置指定时间, 指定时间一道便会触发可读事件
    timespec ts{};
    memset(&ts, 0, sizeof(ts));
    ts.tv_sec = interval / 1000;
    ts.tv_nsec = (interval % 1000) * 1000000;

    itimerspec value{};
    memset(&value, 0, sizeof(value));
    value.it_value = ts;

    int ret = timerfd_settime(mFd, 0, &value, nullptr); // 指定时间一道便会触发可读事件
    if(ret != 0)
        ERRORLOG("timerfd_settime error, errno=%d, error=%s", errno, strerror(errno))
    DEBUGLOG("timer reset to %s", ConvertMillisToDateTime(nowTime + interval).c_str())
}

}