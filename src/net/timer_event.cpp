#include "timer_event.h"

namespace ythe {

TimerEvent::TimerEvent(int interval, bool isRepeated, std::function<void()> task)
    :mInterval(interval), mIsRepeated(isRepeated), mTask(task)
{
    mArriveTime = GetNowMs() + mInterval;
}

}