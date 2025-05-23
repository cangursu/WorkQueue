

#include <WorkQueue.h>

#include <iostream>




class WQPApp : public WorkQueuePool<uint64_t, WQPApp>
{
    public:
        WQPApp();
        int  Pop(uint64_t *pData);
        void Begin()   { /*std::cout << "Begin" << std::endl;*/ }
        void End()     { /*std::cout << "End" << std::endl;  */ }
};

WQPApp::WQPApp()
    : WorkQueuePool<uint64_t, WQPApp>(5)
{
}

int WQPApp::Pop(uint64_t *pData)
{
    std::cout << "Pop : do some queued and paralized jobs. " <<  (int) (*pData) << "\n";
    return 0;
}



int main(int argc, const char * argv[])
{
    std::cout << "Hello..............  " << std::endl;

    WQPApp que;
    que.Init(WQ_QUEUE_STATE::WORKING);

    for (int i = 0; i < 10000; ++i)
    {
        que.PushBack(i);
    }

    que.Release();
    return 0;
}
