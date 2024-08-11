#include "utils.h"
#include <sys/epoll.h>
#include <random>
#include <string>
#include <ctime>

namespace ythe {

// 保存进程号和线程号，减少系统调用
static pid_t gPid = 0;
static thread_local pid_t tThreadId = 0;

pid_t GetPid() {
    if (!gPid)
        gPid = getpid();
    return gPid;
}

pid_t GetThreadId() {
    if (!tThreadId)
        tThreadId = static_cast<pid_t>(syscall(SYS_gettid));
    return tThreadId;
}

std::string GetCurrentDateTime() {
    // 获取当前时间点，转换为 tm 结构体
    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm* now_tm = std::localtime(&now_time_t);

    // 格式化时间
    std::ostringstream oss;
    oss << std::put_time(now_tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

int64_t GetNowMs() {
    auto now = std::chrono::system_clock::now();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    return millis;
}

std::string ConvertMillisToDateTime(long long millis) {
    // 将毫秒数转换为系统时钟时间点
    std::chrono::milliseconds ms(millis);
    std::chrono::system_clock::time_point tp(ms);

    // 将时间点转换为 time_t 类型，将 time_t 转换为 tm 结构体
    std::time_t tt = std::chrono::system_clock::to_time_t(tp);
    std::tm* local_tm = std::localtime(&tt);

    // 使用字符串流格式化日期时间
    std::ostringstream oss;
    oss << std::put_time(local_tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string GetMsgID(int length) {
    const std::string digits = "0123456789";
    std::string id;
    id.reserve(length);

    std::mt19937 generator(static_cast<unsigned long>(std::time(nullptr)));
    std::uniform_int_distribution<> distribution(0, digits.size() - 1);

    for (int i = 0; i < length; ++i) {
        id += digits[distribution(generator)];
    }

    return id;
}

uint32_t GetInt32FromNetByte(const char* buf) {
    uint32_t re;
    memcpy(&re, buf, sizeof(re));
    return ntohl(re);
}

std::string EpollEventsToString(const epoll_event& epolleEvent) {
    int events = epolleEvent.events;
    std::ostringstream oss;

    if (events & EPOLLIN) oss << "EPOLLIN ";
    if (events & EPOLLOUT) oss << "EPOLLOUT ";
    if (events & EPOLLPRI) oss << "EPOLLPRI ";
    if (events & EPOLLERR) oss << "EPOLLERR ";
    if (events & EPOLLHUP) oss << "EPOLLHUP ";
    if (events & EPOLLET) oss << "EPOLLET ";
    if (events & EPOLLONESHOT) oss << "EPOLLONESHOT ";

    std::string result = oss.str();
    if (!result.empty()) {
        // Remove the trailing space
        result.pop_back();
    }
    return result;
}


}