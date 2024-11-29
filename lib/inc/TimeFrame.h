
#ifndef __TIME_FRAME_H__
#define __TIME_FRAME_H__


#include <algorithm>
#include <climits>
#include <iomanip>
#include <sstream>
#include <string>
#include <array>
#include <time.h>




// Time Units Conversion Macros

#define US_TO_NS(v)     ((v)*1000L)             // Microsecond to Nanosecond
#define MS_TO_NS(v)     ((v)*1000000L)          // Milisecond to Nanosecond
#define SEC_TO_NS(v)    ((v)*1000000000L)       // Second to Nanosecond
#define MIN_TO_NS(v)    ((v)*60000000000L)      // Minute to Nanosecond
#define HR_TO_NS(v)     ((v)*3600000000000L)    // Hour to Nanosecond

#define NS_TO_US(v)     ((v)/1000.0)            // Nanosecond to Microsecond
#define MS_TO_US(v)     ((v)*1000L)             // Milisecond to Microsecond
#define SEC_TO_US(v)    ((v)*1000000L)          // Second to Microsecond
#define MIN_TO_US(v)    ((v)*60000000L)         // Minute to Microsecond
#define HR_TO_US(v)     ((v)*3600000000L)       // Hour to Microsecond

#define NS_TO_MS(v)     ((v)/1000000.0)         // Nanosecond to Milisecond
#define US_TO_MS(v)     ((v)/1000.0)            // Microsecond to Milisecond
#define SEC_TO_MS(v)    ((v)*1000L)             // Second to Milisecond
#define MIN_TO_MS(v)    ((v)*60000L)            // Minute to Milisecond
#define HR_TO_MS(v)     ((v)*3600000L)          // Hour to Milisecond

#define NS_TO_SEC(v)    ((v)/1000000000.0)      // Nanosecond to Second
#define US_TO_SEC(v)    ((v)/1000000.0)         // Microsecond to Second
#define MS_TO_SEC(v)    ((v)/1000.0)            // Milisecond to Second
#define MIN_TO_SEC(v)   ((v)/60.0)              // Minute to Second
#define HR_TO_SEC(v)    ((v)/3600.0)            // Hour to Second

#define NS_TO_MIN(v)    ((v)/60000000000.0)     // Nanosecondto Minute
#define US_TO_MIN(v)    ((v)/60000000.0)        // Microsecondto Minute
#define MS_TO_MIN(v)    ((v)/60000.0)           // Milisecondto Minute
#define SEC_TO_MIN(v)   ((v)/60.0)              // Second to Minute
#define HR_TO_MIN(v)    ((v)*60L)               // Hour to Minute

#define NS_TO_HR(v)     ((v)/3600000000000.0    // Nanosecondto to Hour
#define US_TO_HR(v)     ((v)/3600000000.0)      // Microsecondto to Hour
#define MS_TO_HR(v)     ((v)/3600000.0)         // Milisecondto to Hour
#define SEC_TO_HR(v)    ((v)/3600.0)            // Second to Hour
#define MIN_TO_HR(v)    ((v)/60.0)              // Minute to Hour



// #define TIMEFRAME(tm, fn, args...) { (tm).Start(); (fn)(args); (tm).Stop(); }
// #define TIMEFRAME_RC(tm, rc, fn, args...) { (tm).Start(); (rc) = (fn)(args); (tm).Stop(); }


//TODO: Convert these to macros
std::string TimespecText(const timespec &ts);
void        TimespecText(const timespec &ts, std::ostringstream &os);
int64_t     TimespecToNs(const timespec &ts);
timespec    TimespecFromNs(double ns);


class TimeFrame
{
    public:
        TimeFrame();
        TimeFrame(const TimeFrame &val) = default;
        TimeFrame(TimeFrame &&val)      = default;
        ~TimeFrame();

        TimeFrame &operator = (const TimeFrame &val) = default;
        TimeFrame &operator = (TimeFrame &&val)      = default;

        bool        operator <  (const TimeFrame& lhs) const  { return TimeFrame::TimeSpecCmp(Elaps(), lhs.Elaps()) < 0;   }
        bool        operator >  (const TimeFrame& lhs) const  { return TimeFrame::TimeSpecCmp(Elaps(), lhs.Elaps()) > 0;   }
        bool        operator == (const TimeFrame& lhs) const  { return TimeFrame::TimeSpecCmp(Elaps(), lhs.Elaps()) == 0;  }


        int         Start();
        int         Stop();
        void        Reset()                     { Start(); Stop();                                          }
        void        Step(const timespec &ts)    { _start = _stop; _stop = ts;                                   }
        void        Step()                      { timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts); Step(ts);   }


        timespec    TimeStamp() const;
        timespec    Elaps() const;
        int64_t     ElapsNs() const;

        std::string ElapsText() const;
        static std::string TimeStampText(const timespec &ts);


        static timespec TimeSpecDif (const timespec &t1, const timespec &t2);
        static timespec TimeSpecAdd (const timespec &t1, const timespec &t2);
        static timespec TimeSpecDiv (const timespec &t1, double val);
        static int      TimeSpecCmp (const timespec &t1, const timespec &t2);

    private :
        timespec    _start {};
        timespec    _stop {};
        timespec    _ts {};
};



inline bool     operator <  (const timespec& lhs, const timespec& rhs)  { return TimeFrame::TimeSpecCmp(lhs, rhs) < 0;      }
inline bool     operator >  (const timespec& lhs, const timespec& rhs)  { return TimeFrame::TimeSpecCmp(lhs, rhs) > 0;      }
inline bool     operator == (const timespec& lhs, const timespec& rhs)  { return TimeFrame::TimeSpecCmp(lhs, rhs) == 0;     }
inline bool     operator >= (const timespec& lhs, const timespec& rhs)  { int cmp = TimeFrame::TimeSpecCmp(lhs, rhs); return (cmp == 0) || (cmp > 0); }
inline bool     operator <= (const timespec& lhs, const timespec& rhs)  { int cmp = TimeFrame::TimeSpecCmp(lhs, rhs); return (cmp == 0) || (cmp < 0); }

inline timespec operator -  (const timespec& lhs, const timespec& rhs)  { return TimeFrame::TimeSpecDif(lhs, rhs);          }
inline timespec operator +  (const timespec& lhs, const timespec& rhs)  { return TimeFrame::TimeSpecAdd(lhs, rhs);          }
inline timespec operator /  (const timespec& lhs, double val)           { return TimeFrame::TimeSpecDiv(lhs, val);          }
inline timespec operator *  (const timespec& lhs, double val)           { return { time_t(lhs.tv_sec * val), long(lhs.tv_nsec * val) };         }

inline timespec operator -= (timespec& lhs,       const timespec& rhs)  { return lhs = TimeFrame::TimeSpecDif(lhs, rhs);    }
inline timespec operator += (timespec& lhs,       const timespec& rhs)  { return lhs = TimeFrame::TimeSpecAdd(lhs, rhs);    }
inline timespec operator /= (timespec& lhs,       double val)           { return lhs = TimeFrame::TimeSpecDiv(lhs, val);    }
inline timespec operator *= (timespec& lhs,       double val)           { return lhs = { time_t(lhs.tv_sec * val), long(lhs.tv_nsec * val) };   }





template <int TryCOUNT>
struct MeasureCollection
{
    std::string _name;
    std::array<TimeFrame, TryCOUNT> _data;
    //timespec    _mean {};

    std::string BenchmarkText();
    std::string BenchmarkTextBrief();
    void        BenchmarkTextBrief(std::ostringstream &ss);
    std::string BenchmarkTextData();
    void        BenchmarkTextData(std::ostringstream &ss);

    timespec    Mean();
    timespec    Median();
    void        MinMax(timespec &min, timespec &max);

    std::array<TimeFrame, TryCOUNT> SortData() const
    {
        std::array<TimeFrame, TryCOUNT> dataSorted = _data;
        //TODO: Research for a high perf sorting alorithm
        std::sort(dataSorted.begin(), dataSorted.end()/*, TimeFrame::TimeSpecCmp*/);
        return dataSorted;
    }
};


template <int TryCOUNT>
std::string MeasureCollection<TryCOUNT>::BenchmarkTextBrief()
{
    std::ostringstream ss;
    BenchmarkTextBrief(ss);
    return ss.str();
}


template <int TryCOUNT>
void MeasureCollection<TryCOUNT>::BenchmarkTextBrief(std::ostringstream &ss)
{
    timespec min, max;
    MinMax(min, max);

    ss  << "NAME        : " << _name                    << std::endl;
    ss  << "  Req Count : " << _data.size()             << std::endl;
    ss  << "  Mean      : " << TimespecText(Mean())     << std::endl;
    ss  << "  Medaian   : " << TimespecText(Median())   << std::endl;
    ss  << "  Min       : " << TimespecText(min)        << std::endl;
    ss  << "  Max       : " << TimespecText(max)        << std::endl;
}


template <int TryCOUNT>
std::string MeasureCollection<TryCOUNT>::BenchmarkText()
{
    std::ostringstream ss;

    BenchmarkTextBrief(ss);
    BenchmarkTextData(ss);

    return ss.str();
}


template <int TryCOUNT>
std::string MeasureCollection<TryCOUNT>::BenchmarkTextData()
{
    std::ostringstream ss;
    BenchmarkTextData(ss);
    return ss.str();
}


template <int TryCOUNT>
void MeasureCollection<TryCOUNT>::BenchmarkTextData(std::ostringstream &ss)
{
    for(int tryIdx = 0; tryIdx < TryCOUNT; ++tryIdx)
    {
        ss  << _data[tryIdx].TimeStampText() << " - "
            << "Proc: " << std::setw(16) << _data[tryIdx].ElapsText() << " "
            << std::endl;
    }
}


template <int TryCOUNT>
timespec MeasureCollection<TryCOUNT>::Mean()
{
    timespec mean {};
    for (auto &item : _data)
    {
        timespec ts = item.Elaps();
        mean += ts;
    }

    return mean /= TryCOUNT;
}


template <int TryCOUNT>
timespec MeasureCollection<TryCOUNT>::Median()
{
    timespec res {};
    auto s = SortData();

    int n = int(TryCOUNT/2.0);
    if (0 == (TryCOUNT % 2))
    {
        timespec t1 = _data[n].Elaps();
        timespec t2 = _data[n + 1].Elaps();
        res = (t1 + t2) / 2.0;
    }
    else
    {
        res = _data[n + 1].Elaps();
    }

    return res;
}


template <int TryCOUNT>
void MeasureCollection<TryCOUNT>::MinMax(timespec &min, timespec &max)
{
    min.tv_sec  = INT_MAX;
    min.tv_nsec = LONG_MAX;
    max.tv_sec  = 0;
    max.tv_nsec = 0;


    for (int i = 0; i < TryCOUNT; i++)
    {
        timespec e = _data[i].Elaps();
        if (e > max)
            max = e;
        if (e < min)
            min = e;
    }
}







inline
TimeFrame::TimeFrame()
{
    Start();
}

/*
inline
TimeFrame::TimeFrame(const TimeFrame &val)
    : _start(val._start)
    , _stop(val._stop)
{
}
*/

inline
TimeFrame::~TimeFrame()
{
    Stop();
}

inline
int TimeFrame::Start()
{
    clock_gettime(CLOCK_REALTIME, &_ts);
    if (clock_gettime(CLOCK_MONOTONIC, &_start) == -1)
        return -1;
    _stop = _start;
    return 0;
}

inline
int TimeFrame::Stop()
{
    if (clock_gettime(CLOCK_MONOTONIC, &_stop) == -1)
        return -1;
    return 0;
}

inline
timespec TimeFrame::TimeStamp() const
{
    return _ts;
}



#endif  /* __TIME_FRAME_H__ */
