
// clang-format off


#ifndef __WORK_QUEUE_H__
#define __WORK_QUEUE_H__

#include "TimeFrame.h"

#include <thread>
#include <sstream>
#include <condition_variable>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <deque>
#include <list>
#include <vector>
#include <iostream>
#include <stdint.h>





/**
 * @brief A base class template implementing the CRTP (Curiously Recurring Template Pattern) for thread management.
 *
 * This class provides common threading functionality including starting, joining,
 * yielding, and precise sleeping. Derived classes must implement a Run() method
 * which will be executed in the spawned thread.
 *
 * Usage example:
 * @code
 * class MyThread : public Thread<MyThread> {
 * public:
 *     void Run() {
 *         // Thread execution code here
 *     }
 * };
 * @endcode
 *
 * @tparam T The derived class type (CRTP pattern)
 */
template <typename T>
class Thread
{
    public:
        virtual ~Thread() = default;
        void    Start()             { _th  = std::thread( [this]()
                                                { static_cast<T *>(this)->Run(); } );   }
        void    Join()              { if (_th.joinable()) _th.join();                   }
        void    Yield()             { std::this_thread::yield();                        }
        void    USleep(uint32_t ns);

    private:
        std::thread     _th;
        timespec        _ts {};
};


template <typename T>
void Thread<T>::USleep(uint32_t ns)
{
    clock_gettime(CLOCK_MONOTONIC, &_ts);

    _ts.tv_nsec += ns;
    if(_ts.tv_nsec >= SEC_TO_NS(1))
    {
        _ts.tv_nsec -= SEC_TO_NS(1);
        _ts.tv_sec++;
    }
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &_ts, NULL);
}





/**
 * @brief A periodic execution thread using the CRTP (Curiously Recurring Template Pattern).
 *
 * This class extends Thread<TickThread<T>> to provide periodic execution of a Tick()
 * method that should be implemented in the derived class. The thread runs at a configurable
 * interval and maintains statistics about execution time and tick count.
 *
 * TickThread calls the derived class's Tick() method at regular intervals until
 * explicitly stopped. It also provides lifecycle hooks via OnBegin() and OnEnd() methods.
 *
 * Usage example:
 * @code
 * class MyTicker : public TickThread<MyTicker> {
 * public:
 *     void Tick() override {
 *         // Code to execute on each tick
 *     }
 *
 *     bool OnBegin() override {
 *         // Custom initialization, return false to prevent execution
 *         return true;
 *     }
 *
 *     void OnEnd() override {
 *         // Custom cleanup
 *     }
 * };
 * @endcode
 *
 * @tparam T The derived class type (CRTP pattern)
 */
template <typename T>
class TickThread : public Thread<TickThread<T>>
{
    public:

        void        Stop();
        void        SetInterval(uint64_t ns);

        uint64_t    TickCount()     { return _tickCount; }
        TimeFrame   TickTimeFrame() { return _tickTs;    }

        bool        DoQuit()        { return _quit; }

//    protected:
        void        Run();

    private:
        uint64_t                    _interval {1000};
//        std::condition_variable     _cv;
//        std::mutex                  _mtx;
        std::atomic_bool            _quit {false};

        std::atomic<uint64_t>       _tickCount  {0};
        TimeFrame                   _tickTs     {};
};




template <typename T>
void TickThread<T>::SetInterval(uint64_t ns)
{
//    std::unique_lock<std::mutex> lock(_mtx);
    _interval = ns;
}


template <typename T>
void TickThread<T>::Stop()
{
    {
//        std::unique_lock<std::mutex> lock(_mtx);
        _quit.store(true);
//        _cv.notify_all();
    }
    Thread<TickThread<T>>::Join();
}


template <typename T>
void TickThread<T>::Run()
{
    _tickCount = 0;
    _tickTs.Reset();

    if (false == static_cast<T *>(this)->OnBegin())
        return;

//    std::unique_lock<std::mutex> lock(_mtx);
//    while (!_cv.wait_for(lock, _interval, [this]{return _quit.load();}))
//    {
//        _tickTs.Step();
//        ++_tickCount;
//
//        Tick();
//    }

    while (false == DoQuit())
    {
        _tickTs.Step();
        ++_tickCount;

        static_cast<T *>(this)->Tick();

        if ((false == DoQuit()) && _interval > 0)
            this->USleep(_interval);
    }

    static_cast<T *>(this)->OnEnd();
}









/**
 * @brief
 *
 */

enum class WQ_QUEUE_STATE
{
    NA              = 0,
    WORKING         = 1,
    PAUSE           = 2,
    EXITING_WAIT    = 3,
    EXITING_FORCE   = 4,
};

std::string WQ_QUEUE_STATE_text(WQ_QUEUE_STATE value);




/**
 * @brief A thread-safe work queue implementation using the CRTP (Curiously Recurring Template Pattern).
 *
 * This class extends Thread<WorkQueue<TData, TDerived>> to provide a thread-safe queue
 * for processing data items of type TData. The derived class should implement Pop() method
 * to process queue items, and optionally Begin() and End() methods for initialization and cleanup.
 *
 * The worker queue manages a collection of data items and processes them in a background thread.
 * It supports various states including WORKING, PAUSE, EXITING_WAIT, and EXITING_FORCE.
 *
 * Usage example:
 * @code
 * class MyWorker : public WorkQueue<MyData, MyWorker> {
 * public:
 *     void Begin() {
 *         // Called when the worker starts
 *     }
 *
 *     void End() {
 *         // Called when the worker ends
 *     }
 *
 *     void Pop(MyData* data) {
 *         // Process the data item
 *     }
 * };
 * @endcode
 *
 * @tparam TData The type of data items to be processed
 * @tparam TDerived The derived class type (CRTP pattern)
 */


template <typename TData, typename TDerived>
class WorkQueue : public Thread<WorkQueue<TData, TDerived>>
{
 public:
    virtual ~WorkQueue();
/*
    enum class QUEUE_STATE
    {
        NA      = 0,
        WORKING = 1,
        EXITING = 2,
        PAUSE   = 3,
    };
*/

    int                 Init(WQ_QUEUE_STATE state, const std::string &name = "");

    void                SetState(WQ_QUEUE_STATE stat);
    WQ_QUEUE_STATE      GetState() const;
    void                SetWaitTime(const timespec &tmsp);
    const timespec &    GetWaitTime();

    size_t              Size() const ;

    //Form Thread
    void                Run();

    size_t              PushBack (TData &&data);
    size_t              PushBack (const TData &data);
    size_t              PushFront(TData &&data);
    size_t              PushFront(const TData &data);
    size_t              PushFresh(TData &&data);
    size_t              PushFresh(const TData &data);

    void*               Listener();
    void                Release(bool bForce = false);

    const std::string&  Name() const;

 private:

    std::string                 _name;
    mutable std::shared_mutex   _thStatLock;
    std::mutex                  _thLockQue;
    std::condition_variable     _thCond;
    WQ_QUEUE_STATE              _thState = WQ_QUEUE_STATE::EXITING_WAIT;
    timespec                    _thWaitTime {1,0};

    std::deque<TData>           _container;
    std::atomic_size_t          _containerSize = 0;
};


template <typename TData, typename TDerived>
WorkQueue<TData, TDerived>::~WorkQueue()
{
    Release();
}


template <typename TData, typename TDerived>
int WorkQueue<TData, TDerived>::Init(WQ_QUEUE_STATE state, const std::string &name /*= ""*/)
{
    _name = name;
    SetState(state);
    this->Start();
    return 0;
}


template <typename TData, typename TDerived>
void WorkQueue<TData, TDerived>::Release(bool bForce /*= false*/)
{
    SetState(bForce ? WQ_QUEUE_STATE::EXITING_FORCE : WQ_QUEUE_STATE::EXITING_WAIT);
    this->Join();
}


template <typename TData, typename TDerived>
WQ_QUEUE_STATE WorkQueue<TData, TDerived>::GetState() const
{
    std::shared_lock<std::shared_mutex> lck(_thStatLock);
    return _thState;
}


template <typename TData, typename TDerived>
void WorkQueue<TData, TDerived>::SetState(WQ_QUEUE_STATE stat)
{
    std::scoped_lock lck(_thStatLock);
    _thState = stat;
    _thCond.notify_one();
}


template <typename TData, typename TDerived>
const timespec  &WorkQueue<TData, TDerived>::GetWaitTime()
{
    std::shared_lock<std::shared_mutex> lck(_thStatLock);
    return _thWaitTime;
}


template <typename TData, typename TDerived>
void WorkQueue<TData, TDerived>::SetWaitTime(const timespec &tmsp)
{
    std::scoped_lock lck(_thStatLock);
    _thWaitTime = tmsp;
}


template <typename TData, typename TDerived>
size_t WorkQueue<TData, TDerived>::Size() const
{
    return _containerSize;
}


template <typename TData, typename TDerived>
const std::string& WorkQueue<TData, TDerived>::Name() const
{
    return _name;
}


template <typename TData, typename TDerived>
size_t WorkQueue<TData, TDerived>::PushBack(const TData &data)
{
    switch (GetState())
    {
        case WQ_QUEUE_STATE::WORKING :
        {
            std::lock_guard<std::mutex> lck{_thLockQue};
            _container.emplace_front(data);
            ++_containerSize;
            _thCond.notify_one();
            break;
        }

        default :
            break;
    }

    return _containerSize;
}


template <typename TData, typename TDerived>
size_t WorkQueue<TData, TDerived>::PushBack(TData &&data)
{
    return PushBack(data);
}


template <typename TData, typename TDerived>
size_t WorkQueue<TData, TDerived>::PushFront(const TData &data)
{
    switch (GetState())
    {
        case WQ_QUEUE_STATE::WORKING :
        {
            std::lock_guard<std::mutex> lck{_thLockQue};
            _container.emplace_back(data);
            ++_containerSize;
            _thCond.notify_one();
        }

        default :
            break;
    }

    return _containerSize;
}


template <typename TData, typename TDerived>
size_t WorkQueue<TData, TDerived>::PushFront(TData &&data)
{
    return PushFront(data);
}


template <typename TData, typename TDerived>
size_t WorkQueue<TData, TDerived>::PushFresh(const TData &data)
{
    switch (GetState())
    {
        case WQ_QUEUE_STATE::WORKING :
        {
            std::lock_guard<std::mutex> lck{_thLockQue};
            _container.clear();
            _container.emplace_back(data);
            _containerSize = 1;
            _thCond.notify_one();
        }

        default :
            break;
    }

    return _containerSize;
}


template <typename TData, typename TDerived>
size_t WorkQueue<TData, TDerived>::PushFresh(TData &&data)
{
    return PushFresh(data);
}


template <typename TData, typename TDerived>
void WorkQueue<TData, TDerived>::Run()
{
//    std::cout << "WorkQueue thread : " << _name << " : Entering\n";
    static_cast<TDerived*>(this)->Begin();
    Listener();
    static_cast<TDerived*>(this)->End();
//    std::cout << "WorkQueue thread : " << _name << " : Quiting \n";
}


template <typename TData, typename TDerived>
void* WorkQueue<TData, TDerived>::Listener()
{
    bool doExit = false;
    for(/*int count = 0*/; true != doExit; /*count++*/)
    {
        switch(GetState())
        {
            case WQ_QUEUE_STATE::WORKING:
            case WQ_QUEUE_STATE::EXITING_WAIT:
            {
                std::list<TData> listBuff;

                {
                    std::unique_lock<std::mutex> lck{_thLockQue};
                    _thCond.wait(lck, [this]()   {  return (GetState() == WQ_QUEUE_STATE::EXITING_FORCE) ||
                                                           (GetState() == WQ_QUEUE_STATE::EXITING_WAIT) ||
                                                           (_containerSize > 0); });
                    //std::cout << "_containerSize : " << _containerSize << std::endl;

                    switch (GetState())
                    {
                        case WQ_QUEUE_STATE::EXITING_FORCE :
                            doExit = true;
                            break;

                        case WQ_QUEUE_STATE::EXITING_WAIT :
                            if (0 == _containerSize)
                            {
                                doExit = true;
                                break;
                            }

                        default:
                            while (_containerSize > 0)
                            {
                                listBuff.push_back(std::move(_container.back()));
                                //Pop(&_container.back());

                                _container.pop_back();
                                _containerSize--;
                            }
                    }
                }

                for(auto &item : listBuff)
                {
                    if (GetState() != WQ_QUEUE_STATE::EXITING_FORCE)
                        static_cast<TDerived*>(this)->Pop(&item);
                }

                break;
            }

            case WQ_QUEUE_STATE::PAUSE:
                clock_nanosleep(CLOCK_MONOTONIC, 0, &GetWaitTime(), NULL);
                break;

            case WQ_QUEUE_STATE::EXITING_FORCE:
                doExit = true;
                break;

//            case WQ_QUEUE_STATE::EXITING_WAIT:
//                if (0 == _containerSize)
//                    doExit = true;
//                break;

            default :
                break;
        }
    }
    SetState(WQ_QUEUE_STATE::NA);

    return NULL;
}







/**
 * @brief A pool of worker queues that distributes work items across multiple worker threads.
 *
 * This class manages a collection of WorkQueue instances to provide parallel processing
 * of data items. It automatically distributes items among the worker queues using a
 * load-balancing approach. The derived class must implement Begin(), Pop(), and End()
 * methods that will be called by each worker queue in the pool.
 *
 * Usage example:
 * @code
 * class MyWorkerPool : public WorkQueuePool<MyData, MyWorkerPool> {
 * public:
 *     MyWorkerPool(size_t queueCount) : WorkQueuePool<MyData, MyWorkerPool>(queueCount)
 *     {
 *     }
 *
 *     void Begin()
 *     {
 *         // Called when each worker starts
 *     }
 *
 *     int Pop(MyData* data)
 *     {
 *         // Process the data item
 *         return 0;
 *     }
 *
 *     void End()
 *     {
 *         // Called when each worker ends
 *     }
 * };
 * @endcode
 *
 * @tparam TData The type of data items to be processed
 * @tparam TDerived The derived class type (CRTP pattern)
 */

template <typename TData, typename TDerived>
class WorkQueuePool
{
    private:
        class WorkQueuePoolItem : public WorkQueue<TData, WorkQueuePoolItem>
        {
            public:
                void SetPool(TDerived *pool)
                {
                    _pPool = pool;
                }
                void Begin()
                {
                    if (nullptr == _pPool)
                    {
                        std::cerr << "ERROR: invalid _pPool" << std::endl;
                        return;
                    }

                    return _pPool->Begin();
                }

                int Pop(TData *data)
                {
                    if (nullptr == _pPool)
                    {
                        std::cerr << "ERROR: invalid _pPool" << std::endl;
                        return -1;
                    }

                    return _pPool->Pop(data);
                }

                void End()
                {
                    if (nullptr == _pPool)
                    {
                        std::cerr << "ERROR: invalid _pPool" << std::endl;
                        return;
                    }

                    return _pPool->End();
                }
            private:
                TDerived *_pPool = nullptr;
        };

    public :

        using WorkQueuePoolList =  std::vector<WorkQueuePoolItem>;

        virtual ~WorkQueuePool() = default;

        WorkQueuePool(size_t queCount)
            : _queCount(queCount)
            , _pool(queCount)
        {
        }

        int             Init(WQ_QUEUE_STATE state, const std::string &name = "");
        void            Release();

        int             PushBack (TData &&data);
        int             PushFront(TData &&data);

        size_t          QueCount() const;
        size_t          Size(std::vector<int> &sizeList);

    private :
        int             MaxIdx();
        int             MinIdx();

        std::string         _name;
        size_t              _queCount = 16;
        WorkQueuePoolList   _pool;
};


template <typename TData, typename TDerived>
int WorkQueuePool<TData, TDerived>::Init(WQ_QUEUE_STATE state, const std::string &name /*= ""*/)
{
    _name       = name;

    for (size_t idx = 0; idx < _queCount; ++idx)
    {
        _pool[idx].SetPool(static_cast<TDerived*>(this));
        _pool[idx].Init(state, name + ":" + std::to_string(idx));
    }
    return 0;
}


template <typename TData, typename TDerived>
void WorkQueuePool<TData, TDerived>::Release()
{
    for (size_t idx = 0; idx < _queCount; ++idx)
        _pool[idx].Release();
}

template <typename TData, typename TDerived>
int WorkQueuePool<TData, TDerived>::MaxIdx()
{
    size_t  sizeMax = 0;
    size_t  idxMax  = (size_t)-1;

    for (size_t idx = 0; idx < _queCount; ++idx)
    {
        size_t size = _pool[idx].Size();
        if (size >= sizeMax)
        {
            sizeMax = size;
            idxMax  = idx;
        }
    }
    return idxMax;
}

template <typename TData, typename TDerived>
int WorkQueuePool<TData, TDerived>::MinIdx()
{
    size_t  sizeMin = (size_t)-1;
    size_t  idxMin  = (size_t)-1;

    for (size_t idx = 0; idx < _queCount; ++idx)
    {
        size_t size = _pool[idx].Size();
        if (size < sizeMin)
        {
            sizeMin = size;
            idxMin  = idx;
        }
    }

    return idxMin;
}


template <typename TData, typename TDerived>
size_t WorkQueuePool<TData, TDerived>::QueCount() const
{
    return  _queCount;
}


template <typename TData, typename TDerived>
size_t WorkQueuePool<TData, TDerived>::Size(std::vector<int> &sizeList)
{
    size_t sum = 0;
    for (size_t idx = 0; idx < _queCount; ++idx)
    {
        size_t sz = _pool[idx].Size();
        sum += sz;
        sizeList.push_back(sz);
    }
    return sum;
}


template <typename TData, typename TDerived>
int WorkQueuePool<TData, TDerived>::PushBack (TData &&data)
{
    int idx = MinIdx();
    if (idx > -1)
        _pool[idx].PushBack(std::move(data));

    return idx;
}


template <typename TData, typename TDerived>
int WorkQueuePool<TData, TDerived>::PushFront(TData &&data)
{
    int idx = MinIdx();
    if (idx > -1)
        _pool[idx].PushFront(std::move(data));

    return idx;
}




#endif // __WORK_QUEUE_H__

// clang-format on

