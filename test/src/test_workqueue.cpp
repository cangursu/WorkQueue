
// clang-format off


#include <WorkQueue.h>

#include <gtest/gtest.h>
#include <atomic>
#include <unistd.h>



TEST(test_workqueue, wq_basicpush)
{
    class WQTester : public WorkQueue<uint8_t, Thread>
    {
        public:
            void Begin()
            {
                _tidBegin = gettid();
            }

            void End()
            {
                _tidEnd = gettid();
            }

            int Pop(uint8_t *pData)
            {
                _tidPop = gettid();

                EXPECT_TRUE(pData);
                if (pData) _data |= *pData;
                return 0;
            }

            uint8_t _data = 0;
            pid_t   _tidBegin = 0;
            pid_t   _tidPop   = 0;
            pid_t   _tidEnd   = 0;
    };

    WQTester que;
    que.Init(WQ_QUEUE_STATE::WORKING);

    for (int i = 0; i < 4; ++i)
    {
        que.PushBack(0x1 << i);
    }

    //Waiting for completion mannually
    for (int countTry = 0; (que._data != 0xF) && (countTry < 10); ++countTry)
        usleep (50);

    EXPECT_EQ(que._data, 0xF);

    EXPECT_NE(que._tidBegin, 0);
    EXPECT_NE(que._tidPop,   0);
    EXPECT_EQ(que._tidBegin, que._tidPop);

    EXPECT_EQ(que._tidEnd,   0);

    que.Release();

    EXPECT_NE(que._tidEnd,   0);
    EXPECT_EQ(que._tidPop,   que._tidEnd);
    EXPECT_EQ(que._tidEnd,   que._tidBegin);
}



TEST(test_workqueue, wq_statetext)
{
    EXPECT_EQ(WQ_QUEUE_STATE_text(WQ_QUEUE_STATE::NA     ), std::string("NA")       );
    EXPECT_EQ(WQ_QUEUE_STATE_text(WQ_QUEUE_STATE::WORKING), std::string("WORKING")  );
    EXPECT_EQ(WQ_QUEUE_STATE_text(WQ_QUEUE_STATE::EXITING), std::string("EXITING")  );
    EXPECT_EQ(WQ_QUEUE_STATE_text(WQ_QUEUE_STATE::PAUSE  ), std::string("PAUSE")    );
}




class TickThreadTest : public TickThread
{
    public:
        virtual void Tick()
        {
        }
};

TEST(test_workqueue, wq_tickthred)
{
    TickThreadTest  tester;

    tester.SetInterval(10);
    tester.Start();
    usleep(100000);
    tester.Stop();
    TimeFrame  timer = tester.TickTimeFrame();

    EXPECT_GE(tester.TickCount(), 8);
    EXPECT_LE(tester.TickCount(), 12);

    timespec ts = timer.Elaps();

    EXPECT_EQ(ts.tv_sec,         0);
    EXPECT_GE(ts.tv_nsec,   7000000);
    EXPECT_LE(ts.tv_nsec,  13000000);

    EXPECT_TRUE (timer._stop  >  timer._start );
    EXPECT_TRUE (timer._start <  timer._stop  );
    EXPECT_TRUE (timer._stop  >= timer._start );
    EXPECT_TRUE (timer._start <= timer._stop  );
    EXPECT_TRUE (timer._start == timer._start );
}



TEST(test_wqpool, wqp_basicpush)
{
    static std::atomic_uint64_t global_data = 0;

    class WQPTester : public WorkQueuePool<uint64_t, Thread>
    {
        public:
            WQPTester(size_t queCount)
                : WorkQueuePool<uint64_t, Thread>(queCount)
            {
            }

            void Begin()
            {
            }

            void End()
            {
            }

            int Pop(uint64_t *pData)
            {
                //std::cout << "Pop : " << std::hex <<  (int) (*pData) << "\n";
                EXPECT_TRUE(pData);
                if (pData) global_data |= *pData;
                return 0;
            }
    };

    global_data = 0;
    WQPTester wpool(4);
    wpool.Init(WQ_QUEUE_STATE::WORKING, "WQPTester");


    for (size_t i = 0; i < (8 * sizeof (global_data)); ++i)
        wpool.PushBack(0x1 << i);

    //Waiting for completion mannually
    uint64_t expectedValue = -1;// 0xFFFFFFFFFFFFFFFF;
    for (int countTry = 0; (global_data != expectedValue) && (countTry < 10); ++countTry)
        usleep (50);
    EXPECT_EQ(global_data, expectedValue);


    wpool.Release();
}

// clang-format on
