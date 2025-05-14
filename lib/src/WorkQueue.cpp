
// clang-format off


#include <WorkQueue.h>



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
