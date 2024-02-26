

#include "TimeFrame.h"

#include <ctime>



std::string TimeFrame::TimeStampText(const timespec &ts)
{
    struct tm tm;
    localtime_r(&ts.tv_sec, &tm);

    char buff[64] = "";

    size_t ps = 0;
    ps =  std::strftime(buff, 64, "%Y-%m-%d %H:%M:%S.", &tm);
    ps += std::sprintf(buff + ps, "%09luZ", ts.tv_nsec);

    return std::string(buff, ps);
}
