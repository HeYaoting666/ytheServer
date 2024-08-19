#pragma once
#include <memory>
#include <functional>
#include "src/utils/utils.h"

namespace ythe {

class TimerEvent {
public:
    typedef std::shared_ptr<TimerEvent> sp;

private:
    int64_t                 mArriveTime = 0;       // 定时器执行时的时间 ms
    int64_t                 mInterval   = 0;       // 定时器执行时间间隔 ms
    bool                    mIsRepeated = false;   // 定时器任务执行是否重复
    bool                    mIsCanceled = false;   // 是否被删除
    std::function<void()>   mTask;                 // 定时器回调函数

public:
    TimerEvent(int interval, bool is_repeated, std::function<void()> task);

public:
    int64_t               GetArriveTime() const { return mArriveTime; }

    std::function<void()> GetCallBack() const { return mTask; }

    void                  SetCancel(bool val)   { mIsRepeated = val; }

    void                  ResetArriveTime()     { mArriveTime = GetNowMs() + mInterval; }

    bool                  IsCancel() const      { return mIsCanceled; }

    bool                  IsRepeated() const    { return mIsRepeated; }
};

}