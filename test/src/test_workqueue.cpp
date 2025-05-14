
// clang-format off


#include <WorkQueue.h>

#include <gtest/gtest.h>
#include <atomic>
#include <unistd.h>



TEST(test_workqueue, wq_basicpush)
{
    class WQTester : public WorkQueue<uint64_t, WQTester>
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

            int Pop(uint64_t *pData)
            {
                _tidPop = gettid();

                EXPECT_TRUE(pData);
                if (pData) _data |= *pData;

                return 0;
            }

            uint64_t    _data     = 0;
            pid_t       _tidBegin = 0;
            pid_t       _tidPop   = 0;
            pid_t       _tidEnd   = 0;
    };

    WQTester que;
    que.Init(WQ_QUEUE_STATE::WORKING);

    for (int i = 0; i < 64; ++i)
    {
        que.PushBack(0x1 << i);
    }

    //Waiting for completion mannually
    for (int countTry = 0; (que._data != 0xffffffffffffffff) && (countTry < 10); ++countTry)
        usleep (50);

    EXPECT_EQ(que._data, 0xffffffffffffffff);

    EXPECT_NE(que._tidBegin, 0);
    EXPECT_NE(que._tidPop,   0);
    EXPECT_EQ(que._tidBegin, que._tidPop);

    EXPECT_EQ(que._tidEnd,   0);

    que.Release();

    EXPECT_NE(que._tidEnd,   0);
    EXPECT_EQ(que._tidPop,   que._tidEnd);
    EXPECT_EQ(que._tidEnd,   que._tidBegin);
}


class WQTesterSlow : public WorkQueue<uint8_t, WQTesterSlow>
{
    public:
        int Pop(uint8_t *pData)
        {
            ++_count;
            usleep(1000);
            return 0;
        }

        void Begin()
        {
            _count = 0;
        }

        void End()
        {
        }
        int     _count    = 0;
};

TEST(test_workqueue, wq_exitwait)
{
    WQTesterSlow que;
    que.Init(WQ_QUEUE_STATE::WORKING);
    constexpr int max = 1000;
    for (int i = 0; i < max; ++i)
        que.PushBack(i);
    que.Release();

    EXPECT_EQ(que._count, max);
}

TEST(test_workqueue, wq_exitforce)
{
    WQTesterSlow que;
    que.Init(WQ_QUEUE_STATE::WORKING);
    constexpr int max = 100000;
    for (int i = 0; i < max; ++i)
        que.PushBack(i);
    //que.SetState(WQ_QUEUE_STATE::EXITING_FORCE);
    que.Release(true);

    EXPECT_NE(que._count, max);
}


TEST(test_workqueue, wq_statetext)
{
    EXPECT_EQ(WQ_QUEUE_STATE_text(WQ_QUEUE_STATE::NA),              std::string("NA")               );
    EXPECT_EQ(WQ_QUEUE_STATE_text(WQ_QUEUE_STATE::WORKING),         std::string("WORKING")          );
    EXPECT_EQ(WQ_QUEUE_STATE_text(WQ_QUEUE_STATE::PAUSE),           std::string("PAUSE")            );
    EXPECT_EQ(WQ_QUEUE_STATE_text(WQ_QUEUE_STATE::EXITING_WAIT),    std::string("EXITING_WAIT")     );
    EXPECT_EQ(WQ_QUEUE_STATE_text(WQ_QUEUE_STATE::EXITING_FORCE),   std::string("EXITING_FORCE")    );
}




class TickThreadTest : public TickThread<TickThreadTest>
{
    public:
        void Tick()
        {
        }

        bool OnBegin()
        {
            return true;
        }

        void OnEnd()
        {
        }
};

TEST(test_workqueue, wq_tickthred)
{
    TickThreadTest  tester;

    tester.SetInterval(10);
    tester.Start();
    usleep(10000);
    tester.Stop();
    TimeFrame  timer = tester.TickTimeFrame();

    EXPECT_GE(tester.TickCount(), 8);
    EXPECT_LE(tester.TickCount(), 12);

    timespec ts = timer.Elaps();

    EXPECT_EQ(ts.tv_sec,         0);
    EXPECT_GE(ts.tv_nsec,   700000);
    EXPECT_LE(ts.tv_nsec,  1300000);

//TODO: Add Getter/Setter for _stop, _stop
/*
    EXPECT_TRUE (timer._stop  >  timer._start );
    EXPECT_TRUE (timer._start <  timer._stop  );
    EXPECT_TRUE (timer._stop  >= timer._start );
    EXPECT_TRUE (timer._start <= timer._stop  );
    EXPECT_TRUE (timer._start == timer._start );
*/
}



class QueFreshTest : public WorkQueue<int, QueFreshTest>
{
    public :
        int Pop(int *pData)
        {
            _list.emplace_back(*pData);
            usleep(10);
            return 0;
        }

        void Begin()
        {
            _list.clear();
        }
        void End()
        {
        }

        std::vector<int> _list;
};


TEST(test_workqueue, wq_pushfresh)
{
    QueFreshTest que;
    que.Init(WQ_QUEUE_STATE::WORKING, "PushFreshTest");

    int i = 1;
    que.PushFresh(i);
    usleep(5);
    while(++i < 10)
        que.PushFresh(i);
    que.Release();

    EXPECT_EQ(2, que._list.size());
    EXPECT_EQ(1, que._list[0]);
    EXPECT_EQ(9, que._list[1]);
}




TEST(test_wqpool, wqp_basicpush)
{
    static std::atomic_uint64_t global_data = 0;

    class WQPTester : public WorkQueuePool<uint64_t, WQPTester>
    {
        public:
            WQPTester(size_t queCount)
                : WorkQueuePool<uint64_t, WQPTester>(queCount)
            {
            }

            void Begin()
            {
//                std::cout << "Begin\n";
            }

            void End()
            {
//                std::cout << "End\n";
            }

            int Pop(uint64_t *pData)
            {
//                std::cout << "Pop : " << std::hex <<  (int) (*pData) << "\n";
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
