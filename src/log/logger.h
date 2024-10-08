#pragma once
#include <vector>
#include <string>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "async_logger.h"
#include "src/net/eventloop.h"
#include "src/net/timer_event.h"


#define DEBUGLOG(str, ...) \
    if (ythe::Logger::GetInstance()->GetLogLevel() && ythe::Logger::GetInstance()->GetLogLevel() <= ythe::DEBUG) \
    { \
    ythe::Logger::GetInstance()->PushLog( GetLogEvent(ythe::DEBUG, std::string(__FILE__), std::to_string(__LINE__)) + \
    ythe::formatString(str, ##__VA_ARGS__) + "\n");\
    } \

#define INFOLOG(str, ...) \
    if (ythe::Logger::GetInstance()->GetLogLevel() && ythe::Logger::GetInstance()->GetLogLevel() <= ythe::INFO) \
    { \
    ythe::Logger::GetInstance()->PushLog( GetLogEvent(ythe::INFO, std::string(__FILE__), std::to_string(__LINE__)) + \
    ythe::formatString(str, ##__VA_ARGS__) + "\n");\
    } \

#define ERRORLOG(str, ...) \
    if (ythe::Logger::GetInstance()->GetLogLevel() && ythe::Logger::GetInstance()->GetLogLevel() <= ythe::ERROR) \
    { \
    ythe::Logger::GetInstance()->PushLog( GetLogEvent(ythe::ERROR, std::string(__FILE__), std::to_string(__LINE__)) + \
    ythe::formatString(str, ##__VA_ARGS__) + "\n");\
    } \


namespace ythe {

enum LogLevel{
    UNKNOWN = 0,
    DEBUG = 1,
    INFO = 2,
    ERROR = 3
};

template<typename... Args>
std::string formatString(const char* str, Args&&... args) {
    size_t size = snprintf(nullptr, 0, str, args...);
    std::string result;
    if(size > 0) {
        result.resize(size);
        snprintf(&result[0], size + 1, str, args...);
    }
    return result;
}

std::string GetLogEvent(LogLevel logLevel, std::string fileName, std::string line);

std::string logLevelToString (LogLevel level);

LogLevel stringToLogLevel(const std::string& strLevel);

class Logger {
private:
    bool                     mIsPrint  = true;
    LogLevel                 mSetLevel = LogLevel::DEBUG;

    std::vector<std::string> mBuffer;
    std::mutex               mMutex;

    AsyncLogger::sp          mpAsyncLogger;
    TimerEvent::sp           mTimerEvent;

public:
    static Logger* GetInstance() { 
        static Logger instance;
        return &instance;
    }
    void Init();

    Logger(const Logger&)         = delete;

    void operator=(const Logger&) = delete;

    ~Logger() { stop(); }

private:
    Logger() = default;

public:
    void               PushLog (const std::string& msg);

    TimerEvent::sp     GetTimeEvent() const { return mTimerEvent; }

    LogLevel           GetLogLevel() const { return mSetLevel; }

private:
    void               stop();
    
    void               syncLoop();
};

}