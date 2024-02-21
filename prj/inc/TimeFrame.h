
#ifndef __TIME_FRAME_H__
#define __TIME_FRAME_H__


#include <string>
#include <sstream>
#include <time.h>



inline bool     operator <  (const timespec& lhs, const timespec& rhs);
inline bool     operator >  (const timespec& lhs, const timespec& rhs);
inline bool     operator == (const timespec& lhs, const timespec& rhs);
inline bool     operator >= (const timespec& lhs, const timespec& rhs);
inline bool     operator <= (const timespec& lhs, const timespec& rhs);
inline timespec operator -  (const timespec& lhs, const timespec& rhs);


constexpr int NANO_SECONDS_IN_SEC = 1000000000;


class TimeFrame
{
    public :
        TimeFrame()                             { Reset();                                                      }

        void         Reset()                    { Start(); Stop();                                              }
        void         Start()                    { clock_gettime(CLOCK_MONOTONIC, &_start);                      }
        void         Stop()                     { clock_gettime(CLOCK_MONOTONIC, &_stop);                       }
        void         Step(const timespec &ts)   { _start = _stop; _stop = ts;                                   }
        void         Step()                     { timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts); Step(ts);   }
        timespec     Elaps()                    { return Elaps(_stop);                                          }
        timespec     Elaps(const timespec &ts)  { return ts - _start;                                           }
        std::string  Str()
        {
            std::ostringstream ss;
            ss <<   "(" << _start.tv_sec << ":" << _start.tv_nsec << ") - " <<
                    "(" << _stop.tv_sec  << ":" << _stop.tv_nsec  << ")" ;
            return ss.str();
        }

        static timespec TimeSpecDiff(const timespec &ts1, const timespec &ts2)
        {
            timespec ts { ts1.tv_sec  - ts2.tv_sec,
                          ts1.tv_nsec - ts2.tv_nsec};
            if (ts.tv_nsec < 0)
            {
                ts.tv_sec--;
                ts.tv_nsec += NANO_SECONDS_IN_SEC;
            }
            return ts;
        }

        static int TimeSpecCmp(const timespec& lhs, const timespec& rhs)
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


        timespec    _start  {0,0};
        timespec    _stop   {0,0};
};



inline bool     operator <  (const timespec& lhs, const timespec& rhs) { return TimeFrame::TimeSpecCmp(lhs, rhs) <  0;                                  }
inline bool     operator >  (const timespec& lhs, const timespec& rhs) { return TimeFrame::TimeSpecCmp(lhs, rhs) >  0;                                  }
inline bool     operator == (const timespec& lhs, const timespec& rhs) { return TimeFrame::TimeSpecCmp(lhs, rhs) == 0;                                  }
inline bool     operator >= (const timespec& lhs, const timespec& rhs) { int cmp = TimeFrame::TimeSpecCmp(lhs, rhs); return (cmp == 0) || (cmp > 0);    }
inline bool     operator <= (const timespec& lhs, const timespec& rhs) { int cmp = TimeFrame::TimeSpecCmp(lhs, rhs); return (cmp == 0) || (cmp < 0);    }

inline timespec operator -  (const timespec& lhs, const timespec& rhs) { return TimeFrame::TimeSpecDiff(lhs, rhs);      }



#endif //__TIME_FRAME_H__