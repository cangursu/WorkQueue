

#include "TimeFrame.h"

#include <sstream>
#include <stdlib.h>
#include <stdio.h>
#include <ctime>



const timespec& TimespecConvertOffset()
{
    static timespec s_tsOffset {};
    if ((0 == s_tsOffset.tv_sec) && (0 == s_tsOffset.tv_nsec))
    {
        timespec tsSrc {};
        timespec tsDst {};
        clock_gettime(CLOCK_REALTIME,  &tsSrc);
        clock_gettime(CLOCK_BOOTTIME, &tsDst);

        s_tsOffset = tsSrc - tsDst;
    }
    return s_tsOffset;
}

timespec TimespecConvertBT_RT(const timespec &ts)
{
    const timespec &tsOfs = TimespecConvertOffset();
    return tsOfs + ts;
}


timespec TimespecConvertRT_BT(const timespec &ts)
{
    const timespec &tsOfs = TimespecConvertOffset();
    return ts - tsOfs;
}


std::string TimespecText(const timespec &ts)
{
    std::ostringstream os;
    TimespecText(ts, os);
    return os.str();
}


void TimespecText(const timespec &ts, std::ostringstream &os)
{
    long sec = ts.tv_sec;
    long msc = NS_TO_MS(ts.tv_nsec);                // nanoseconds to millisecond
    long nsc = ts.tv_nsec - (msc * NS_TO_MS(1));    // remaining nanoseconds

    os << sec << ":" << msc << ":" << nsc ;
}


std::string TimespecText2(const timespec &ts, bool date /*= true*/)
{
    std::ostringstream os;
    TimespecText2(ts, os, date);
    return os.str();
}


void TimespecText2(const timespec &ts, std::ostringstream &os, bool date /*= true*/)
{
    tm tmNow {};

    gmtime_r(&ts.tv_sec, &tmNow);
    std::ostringstream ss;
    if(date)
    {
        os  << std::setfill('0') << std::setw(4) << (tmNow.tm_year + 1900) << "."
            << std::setfill('0') << std::setw(2) << tmNow.tm_mon + 1 << "."
            << std::setfill('0') << std::setw(2) << tmNow.tm_mday;
        os  << " - ";
    }

    uint64_t ns = ts.tv_nsec;
    uint64_t ms = NS_TO_MS(ns);
    ns -= MS_TO_NS(ms);
    uint64_t us = NS_TO_US(ns);
    ns -= US_TO_NS(us);

    os  << std::setfill('0') << std::setw(2) << tmNow.tm_hour << ":"
        << std::setfill('0') << std::setw(2) << tmNow.tm_min << ":"
        << std::setfill('0') << std::setw(2) << tmNow.tm_sec << "."
//        << "-" << ts.tv_nsec << "-"
        << std::setfill('0') << std::setw(3) << ms << "."
        << std::setfill('0') << std::setw(3) << us << "."
        << std::setfill('0') << std::setw(3) << ns;
}


int64_t TimespecToNs(const timespec &ts)
{
    return SEC_TO_NS(ts.tv_sec) + ts.tv_nsec;
}


timespec TimespecFromNs(double ns)
{
    timespec ts;
    ts.tv_sec  = NS_TO_SEC(ns);
    ts.tv_nsec = uint64_t(ns) - SEC_TO_NS(ts.tv_sec);

    return ts;
}


timespec TimespecFromNs(uint64_t ns)
{
    timespec ts;
    ts.tv_sec  = NS_TO_SEC(ns);
    ts.tv_nsec = ns - SEC_TO_NS(ts.tv_sec);

    return ts;
}


timespec TimeFrame::Elaps() const
{
    return TimeSpecDif(_stop, _start);
}


int64_t TimeFrame::ElapsNs() const
{
    timespec ts = Elaps();
    return TimespecToNs(ts);
    //return ts.tv_sec * SC_TO_NS + ts.tv_nsec;
}


std::string  TimeFrame::ElapsText() const
{
    timespec ts = Elaps();
    return TimespecText(ts);
}


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


timespec TimeFrame::TimeSpecAdd(const timespec &t1, const timespec &t2)
{
    timespec ts;
    ts.tv_sec  = t2.tv_sec + t1.tv_sec  ;
    ts.tv_nsec = t2.tv_nsec + t1.tv_nsec;

    long q = static_cast<int>NS_TO_SEC(ts.tv_nsec);
    long r = ts.tv_nsec - SEC_TO_NS(q);
    ts.tv_sec += q;
    ts.tv_nsec = r;

    return ts;
}


timespec TimeFrame::TimeSpecDif(const timespec &t1, const timespec &t2)
{
    timespec diff = {   .tv_sec  = t1.tv_sec  - t2.tv_sec,
                        .tv_nsec = t1.tv_nsec - t2.tv_nsec      };
    if (diff.tv_nsec < 0)
    {
        diff.tv_nsec += 1000000000L;
        diff.tv_sec--;
    }

    return diff;
}


timespec TimeFrame::TimeSpecDiv (const timespec &ts, double val)
{
    uint64_t ns = TimespecToNs(ts);
    ns /= val;
    return TimespecFromNs(ns);
}


/*
timespec TimeFrame::TimeSpecDif(const timespec &ts1, const timespec &ts2)
{
    timespec ts { .tv_sec  = ts1.tv_sec - ts2.tv_sec,
                  .tv_nsec = ts1.tv_nsec - ts2.tv_nsec};
    if (ts.tv_nsec < 0)
    {
        ts.tv_sec--;
        ts.tv_nsec += SC_TO_NS;
    }
    return ts;
}
*/


int TimeFrame::TimeSpecCmp(const timespec& lhs, const timespec& rhs)
{
    if (lhs.tv_sec == rhs.tv_sec)
    {
        if (lhs.tv_nsec == rhs.tv_nsec)
            return 0;
        else
            return lhs.tv_nsec < rhs.tv_nsec?-1:+1;
    }
    else
    {
        return lhs.tv_sec < rhs.tv_sec?-1:+1;
    }
}

