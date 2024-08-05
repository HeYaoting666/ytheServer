#include "timer_event.h"
#include "../log/logger.h"

namespace ythe {

TimerEvent::TimerEvent(int interval, bool isRepeated, std::function<void()> task)
    :mInterval(interval), mIsRepeated(isRepeated), mTask(task)
{
    mArriveTime = GetNowMs() + mInterval;
    DEBUGLOG("success create timer event, will execute at [%s]", ConvertMillisToDateTime(mArriveTime).c_str())
}

}