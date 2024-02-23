
// clang-format off


#include <WorkQueue.h>


void TickThread::SetInterval(size_t val)
{
    std::unique_lock<std::mutex> lock(_mtx);
    _interval = std::chrono::milliseconds(val);
}


void TickThread::Stop()
{
    {
        std::unique_lock<std::mutex> lock(_mtx);
        _quit.store(true);
        _cv.notify_all();
    }
    Join();
}


void TickThread::Run()
{
    _tickCount = 0;
    _tickTs.Reset();

    std::unique_lock<std::mutex> lock(_mtx);
    while (!_cv.wait_for(lock, _interval, [this]{return _quit.load();}))
    {
        _tickTs.Step();
        ++_tickCount;

        Tick();
    }
}




std::string WQ_QUEUE_STATE_text(WQ_QUEUE_STATE value)
{
    switch (value)
    {
        case WQ_QUEUE_STATE::NA             : return "NA";
        case WQ_QUEUE_STATE::WORKING        : return "WORKING";
        case WQ_QUEUE_STATE::EXITING_WAIT   : return "EXITING_WAIT";
        case WQ_QUEUE_STATE::EXITING_FORCE  : return "EXITING_FORCE";
        case WQ_QUEUE_STATE::PAUSE          : return "PAUSE";
    }
    return "NA";
}



// clang-format on
