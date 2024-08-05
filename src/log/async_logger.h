#pragma once

#include <vector>
#include <string>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace ythe {

class AsyncLogger {
public:
    typedef std::shared_ptr<AsyncLogger> sp;

private:
    std::thread               mThread;
    std::string               mFileName;
    std::string               mFilePath;
    std::string               mDate;
    int                       mNo = 0;
    int                       mFileMaxSize;
    FILE*                     mFile = nullptr;

    bool                      mTheadStart = false;
    bool                      mIsStop     = false;

    std::mutex                mMutex;
    std::condition_variable   mCondition;
    std::queue<std::vector<std::string>> mBlockQue;

public:
    AsyncLogger(const std::string& fileName, const std::string filePath, int fileMaxSize);

    ~AsyncLogger();

public:
    void PushLogBuffer(const std::vector<std::string>& vec);

    void Flush() { if(mFile) fflush(mFile); }

    void Stop()  { mIsStop = true; }

private:
    void loop();
};

}