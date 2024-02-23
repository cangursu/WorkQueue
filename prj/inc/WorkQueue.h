
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





/**
 * @brief
 * Thread base for Threading/Queueing interfaces.
 *
 */

class Thread
{
    public:
        void            Start() {   _th = std::thread( [this]() { Run(); } );   }
        void            Join()  {   if (_th.joinable()) _th.join();             }
        virtual void    Run()  = 0;
    private:
        std::thread     _th;
};





/**
 * @brief
 * A timer-like frequently triggered Thread interface.
 *
 */

class TickThread : public Thread
{
    public:

        void            Stop();
        void            SetInterval(size_t val);

        virtual void    Tick() = 0;

        uint64_t        TickCount()         { return _tickCount; }
        TimeFrame       TickTimeFrame()     { return _tickTs;    };

    protected:
        virtual void    Run() override;

    private:
        std::chrono::milliseconds   _interval {1000};
        std::condition_variable     _cv;
        std::mutex                  _mtx;
        std::atomic_bool            _quit {false};

        std::atomic<uint64_t>       _tickCount  {0};
        TimeFrame                   _tickTs     {};
};





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
 * @brief
 * WorkQueue, A Queue implementation which is attached with a Worker thread.
 *
 * @tparam TData The Data structure to queued
 * @tparam TThread The Thread Implamantation
 */

template <typename TData, typename TThread>
class WorkQueue : public TThread
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

    int                         Init(WQ_QUEUE_STATE state, const std::string &name = "");

    void                        SetState(WQ_QUEUE_STATE stat);
    WQ_QUEUE_STATE              GetState() const;
    void                        SetWaitTime(const timespec &tmsp);
    const timespec &            GetWaitTime();

    size_t                      Size() const ;

    //Form Thread
    virtual void                Run() override;


    virtual void                Begin();
    virtual int                 Pop(TData *data);
    virtual void                End();
    virtual size_t              PushBack (TData &&data);
    virtual size_t              PushFront(TData &&data);

    virtual void*               Listener();
    void                        Release(bool bForce = false);

    const std::string&          Name() const;

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


template <typename TData, typename TThread>
WorkQueue<TData, TThread>::~WorkQueue()
{
    Release();
}


template <typename TData, typename TThread>
int WorkQueue<TData, TThread>::Pop(TData*)
{
    return 0;
}


template <typename TData, typename TThread>
void WorkQueue<TData, TThread>::Begin()
{
//    std::cout << "Listen thread will be begun\n";
//    std::cout << "Please overload the virtual member function below\n";
//    std::cout << "WorkQueue::Begin\n";
}


template <typename TData, typename TThread>
void WorkQueue<TData, TThread>::End()
{
//    std::cout << "Listen thread will be stop\n";
//    std::cout << "Please overload the virtual member function below\n";
//    std::cout << "WorkQueue::End\n";
}


template <typename TData, typename TThread>
int WorkQueue<TData, TThread>::Init(WQ_QUEUE_STATE state, const std::string &name /*= ""*/)
{
    _name = name;
    SetState(state);
    TThread::Start();
    return 0;
}


template <typename TData, typename TThread>
void WorkQueue<TData, TThread>::Release(bool bForce /*= false*/)
{
    SetState(bForce ? WQ_QUEUE_STATE::EXITING_FORCE : WQ_QUEUE_STATE::EXITING_WAIT);
    TThread::Join();
}


template <typename TData, typename TThread>
/*typename WorkQueue<TData, TThread>::QUEUE_STATE*/
WQ_QUEUE_STATE WorkQueue<TData, TThread>::GetState() const
{
    std::shared_lock<std::shared_mutex> lck(_thStatLock);
    return _thState;
}


template <typename TData, typename TThread>
void WorkQueue<TData, TThread>::SetState(WQ_QUEUE_STATE stat)
{
    std::scoped_lock lck(_thStatLock);
    _thState = stat;
    _thCond.notify_one();
}


template <typename TData, typename TThread>
const timespec  &WorkQueue<TData, TThread>::GetWaitTime()
{
    std::shared_lock<std::shared_mutex> lck(_thStatLock);
    return _thWaitTime;
}


template <typename TData, typename TThread>
void WorkQueue<TData, TThread>::SetWaitTime(const timespec &tmsp)
{
    std::scoped_lock lck(_thStatLock);
    _thWaitTime = tmsp;
}


template <typename TData, typename TThread>
size_t WorkQueue<TData, TThread>::Size() const
{
    return _containerSize;
}


template <typename TData, typename TThread>
const std::string& WorkQueue<TData, TThread>::Name() const
{
    return _name;
}


template <typename TData, typename TThread>
size_t WorkQueue<TData, TThread>::PushBack(TData &&data)
{
    switch (GetState())
    {
        case WQ_QUEUE_STATE::EXITING_WAIT  :
        case WQ_QUEUE_STATE::EXITING_FORCE :
        case WQ_QUEUE_STATE::PAUSE         :
            break;

        default :
            std::lock_guard<std::mutex> lck{_thLockQue};
            _container.emplace_front(std::move(data));
            ++_containerSize;
            _thCond.notify_one();
            break;
    }

    return _containerSize;
}


template <typename TData, typename TThread>
size_t WorkQueue<TData, TThread>::PushFront(TData &&data)
{
    switch (GetState())
    {
        case WQ_QUEUE_STATE::EXITING_WAIT  :
        case WQ_QUEUE_STATE::EXITING_FORCE :
        case WQ_QUEUE_STATE::PAUSE         :
            break;

        default :
            std::lock_guard<std::mutex> lck{_thLockQue};
            _container.emplace_back(std::move(data));
            ++_containerSize;
            _thCond.notify_one();
    }

    return _containerSize;
}

template <typename TData, typename TThread>
void WorkQueue<TData, TThread>::Run()
{
//    std::cout << "WorkQueue thread : " << _name << " : Entering\n";
    Begin();
    Listener();
    End();
//    std::cout << "WorkQueue thread : " << _name << " : Quiting \n";
}


template <typename TData, typename TThread>
void* WorkQueue<TData, TThread>::Listener()
{
    bool doExit = false;
    for(int count = 0; true != doExit; count++)
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
                        Pop(&item);
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
 * @brief
 * WorkQueuePool : A pool of WorkQueue
 *
 * @tparam TData
 * @tparam TThread
 */

template <typename TData, typename TThread>
class WorkQueuePool
{
    class WorkQueuePoolItem : public WorkQueue<TData, TThread>
    {
        public:
            void SetPool(WorkQueuePool *pool)
            {
                _pPool = pool;
            }
            virtual void Begin() override
            {
                if (nullptr == _pPool)
                {
                    std::cerr << "ERROR: invalid _pPool" << std::endl;
                    return;
                }

                return _pPool->Begin();
            }
            virtual int Pop(TData *data) override
            {
                if (nullptr == _pPool)
                {
                    std::cerr << "ERROR: invalid _pPool" << std::endl;
                    return -1;
                }

                return _pPool->Pop(data);
            }
            virtual void End() override
            {
                if (nullptr == _pPool)
                {
                    std::cerr << "ERROR: invalid _pPool" << std::endl;
                    return;
                }

                return _pPool->End();
            }
        private:
            WorkQueuePool *_pPool = nullptr;
    };

    public :

        using WorkQueuePoolList =  std::vector<WorkQueuePoolItem>;

        WorkQueuePool(size_t queCount)
            : _queCount(queCount)
            , _pool(queCount)
        {
        }

        int             Init(/*WorkQueue<TData, TThread>::QUEUE_STATE*/WQ_QUEUE_STATE state, const std::string &name = "");
        void            Release();

        virtual int     Pop(TData *data);
        virtual void    Begin();
        virtual void    End();
        virtual int     PushBack (TData &&data);
        virtual int     PushFront(TData &&data);

        size_t          QueCount() const;
        size_t          Size(std::vector<int> &sizeList);

    private :
        int             MaxIdx();
        int             MinIdx();

        std::string         _name;
        size_t              _queCount = 16;
        WorkQueuePoolList   _pool;
};


template <typename TData, typename TThread>
int WorkQueuePool<TData, TThread>::Init(/*WorkQueue<TData, TThread>::QUEUE_STATE*/WQ_QUEUE_STATE state, const std::string &name /*= ""*/)
{
    _name       = name;

    for (size_t idx = 0; idx < _queCount; ++idx)
    {
        _pool[idx].SetPool(this);
        _pool[idx].Init(state, name + ":" + std::to_string(idx));
    }
    return 0;
}


template <typename TData, typename TThread>
void WorkQueuePool<TData, TThread>::Release()
{
    for (size_t idx = 0; idx < _queCount; ++idx)
        _pool[idx].Release();
}

template <typename TData, typename TThread>
int WorkQueuePool<TData, TThread>::MaxIdx()
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

template <typename TData, typename TThread>
int WorkQueuePool<TData, TThread>::MinIdx()
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


template <typename TData, typename TThread>
size_t WorkQueuePool<TData, TThread>::QueCount() const
{
    return  _queCount;
}


template <typename TData, typename TThread>
size_t WorkQueuePool<TData, TThread>::Size(std::vector<int> &sizeList)
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


template <typename TData, typename TThread>
int WorkQueuePool<TData, TThread>::PushBack (TData &&data)
{
    int idx = MinIdx();
    if (idx > -1)
        _pool[idx].PushBack(std::move(data));

    return idx;
}


template <typename TData, typename TThread>
int WorkQueuePool<TData, TThread>::PushFront(TData &&data)
{
    int idx = MinIdx();
    if (idx > -1)
        _pool[idx].PushFront(std::move(data));

    return idx;
}


template <typename TData, typename TThread>
int WorkQueuePool<TData, TThread>::Pop(TData * /*data*/)
{
    std::cout << "ERROR\n";
    std::cout << "Please overload the virtual member function below\n";
    std::cout << "WorkQueuePool::Pop\n";
    return 0;
}


template <typename TData, typename TThread>
void WorkQueuePool<TData, TThread>::Begin()
{
//    std::cout << "Listen thread will be begun\n";
//    std::cout << "Please overload the virtual member function below\n";
//    std::cout << "WorkQueuePool::Begin\n";
}


template <typename TData, typename TThread>
void WorkQueuePool<TData, TThread>::End()
{
//    std::cout << "Listen thread will be stop\n";
//    std::cout << "Please overload the virtual member function below\n";
//    std::cout << "WorkQueuePool::End\n";
}




#endif // __WORK_QUEUE_H__

// clang-format on

