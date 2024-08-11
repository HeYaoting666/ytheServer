#pragma once

#include <chrono>
#include <ctime>
#include <iomanip>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/epoll.h>
#include <sys/syscall.h>

namespace ythe {

pid_t       GetPid();

pid_t       GetThreadId();

std::string GetCurrentDateTime();

int64_t     GetNowMs();

std::string GetMsgID(int length = 20);

std::string ConvertMillisToDateTime(long long millis);

uint32_t    GetInt32FromNetByte(const char* buf);

std::string EpollEventsToString(const epoll_event& epolleEvent);
}