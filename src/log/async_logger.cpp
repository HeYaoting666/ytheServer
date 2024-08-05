#include "async_logger.h"

namespace ythe {

AsyncLogger::AsyncLogger(const std::string &fileName, const std::string filePath, int fileMaxSize)
    : mFileName(fileName), mFilePath(filePath), mFileMaxSize(fileMaxSize)
{
    mThread = std::thread(&AsyncLogger::loop, this); // 创建写日志线程

    // 等待线程创建完成
    std::unique_lock<std::mutex> lock(mMutex);
    mCondition.wait(lock, [this]() { return mTheadStart; });
}

AsyncLogger::~AsyncLogger()
{
    if (!mIsStop) {
        mIsStop = true;
        mCondition.notify_one();
    }

    if (mThread.joinable()) {
        mThread.join();
    }

    if (mFile) {
        fclose(mFile);
    }
}

void AsyncLogger::PushLogBuffer(const std::vector<std::string> &vec)
{
    {
        std::lock_guard<std::mutex> lock(mMutex);
        mBlockQue.push(vec);
    }
    mCondition.notify_one(); // 唤醒异步日志线程
}

void AsyncLogger::loop() {
    {
        std::unique_lock<std::mutex> lock(mMutex);
        mTheadStart = true;
    }
    mCondition.notify_one();

    while(!mIsStop) {
        std::vector<std::string> logBuffer;
        {
            std::unique_lock<std::mutex> lock(mMutex);
            mCondition.wait(lock, [this]() {return !mBlockQue.empty() || mIsStop;});

            logBuffer = std::move(mBlockQue.front());
            mBlockQue.pop();
        }

        if(!logBuffer.empty()) {
            // 获取当前时间
            auto now = std::chrono::system_clock::now();
            std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
            std::tm* nowTm = std::localtime(&nowTime);
            char date[32];
            std::strftime(date, sizeof(date), "%Y%m%d", nowTm);
            std::string currentDate(date);

            // 判断日期是否相同，不相同写入新文件，否则在原先的 mFIle 中写入日志
            if(currentDate != mDate) {
                mNo = 0;
                mDate = currentDate;
                if(mFile != nullptr) {
                    fclose(mFile);
                }
                std::string logFileName = mFilePath + mFileName + "_" + mDate + "_log.";
                mFile = fopen(logFileName.c_str(), "a");
            }

            // 判断文件大小是否达到上限，若达到上限则文件序号加 1 写入新文件中，否则在原先的 mFIle 中写入日志
            if(ftell(mFile) >= mFileMaxSize) {
                fclose(mFile);
                std::string logFileName = mFilePath + mFileName + "_" + mDate + "_log." + std::to_string(++mNo);
                mFile = fopen(logFileName.c_str(), "a");
            }

            // 将日志内容写入目标文件
            for (const auto& log : logBuffer) {
                if (!log.empty()) {
                    fwrite(log.c_str(), 1, log.length(), mFile);
                }
            }
            fflush(mFile);
        }
    }
}

} // namespace ythe