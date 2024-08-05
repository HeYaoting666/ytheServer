#include <sstream>
#include <memory>
#include <cstring>
#include <cassert>
#include <csignal>
#include <sys/time.h>
#include "logger.h"
#include "../config/config.h"
#include "../utils/utils.h"


namespace ythe {

void Logger::Init()
{
    mIsPrint = Config::GetInstance()->mIsPrintLog;
    mSetLevel = stringToLogLevel(Config::GetInstance()->mLogLevel);
    if (mIsPrint) // 如果设置了打印日志选项，则不启动异步写日志线程
        return;
    
    mpAsyncLogger = std::make_shared<AsyncLogger>(
        Config::GetInstance()->mLogFileName + "_rpc",
        Config::GetInstance()->mLogFilePath,
        Config::GetInstance()->mLogMaxFileSize
    );
}

void Logger::Stop()
{
    syncLoop();
    mpAsyncLogger->Stop();
    mpAsyncLogger->Flush();
}

std::string GetLogEvent(LogLevel logLevel)
{
    std::string nowTimeStr = GetCurrentDateTime();

    pid_t pid = GetPid();
    pid_t threadId = GetThreadId();

    std::stringstream ss;
    ss << "[" << logLevelToString(logLevel) << "]\t"
       << "[" << nowTimeStr << "]\t"
       << "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t"
       << "[" << "Pid: " << pid << ", " << "ThreadId: " << threadId << "]\t";

    return ss.str();
}

void Logger::PushLog(const std::string& msg)
{
    if(mIsPrint) {
        printf("%s\n", msg.c_str());
        return;
    }

    {
        std::lock_guard<std::mutex> lock(mMutex);
        mBuffer.push_back(msg);
    }
}

void Logger::syncLoop()
{
    std::vector<std::string> tmp;
    {
        std::lock_guard<std::mutex> lock(std::mutex);
        tmp.swap(mBuffer);
    }
    if(!tmp.empty()) {
        mpAsyncLogger->PushLogBuffer(tmp);
    }
}

LogLevel stringToLogLevel(const std::string& strLevel) 
{
    if (strLevel == "DEBUG") {
        return DEBUG;
    } else if (strLevel == "INFO") {
        return INFO;
    } else if (strLevel == "ERROR") {
        return ERROR;
    } else {
        return UNKNOWN;
    }
}

std::string logLevelToString(LogLevel level)
{
    switch (level) {
        case DEBUG:
            return "DEBUG";
        case INFO:
            return "INFO";
        case ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}

}