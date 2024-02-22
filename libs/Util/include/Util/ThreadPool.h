#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <chrono>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace CameraKiosk
{
    namespace Util
    {
        class ThreadPool
        {
        public:
            // ThreadPool(size_t num_threads);
            ThreadPool(size_t num_threads)
                : _threadCnt(num_threads), _fStopAll(false)
            {
                _workers.reserve(_threadCnt);
                for (size_t i = 0; i < _threadCnt; ++i)
                {
                    _workers.emplace_back([this]()
                                          { this->WorkerThread(); });
                }
            };
            // ~ThreadPool();
            ~ThreadPool()
            {
                _fStopAll = true;
                _cvTaskQ.notify_all();

                for (auto &t : _workers)
                {
                    t.join();
                }
            }

            template <class F, class... Args>
            std::future<typename std::invoke_result_t<F(Args...)>>
            EnqueueTask(F &&f, Args &&...args);

        private:
            // 총 Worker 쓰레드의 개수.
            size_t _threadCnt;
            // Worker 쓰레드를 보관하는 벡터.
            std::vector<std::thread> _workers;
            // 할일들을 보관하는 Task 큐.
            std::queue<std::function<void()>> _tasks;
            // 위의 Task 큐를 위한 cv 와 mutex.
            std::condition_variable _cvTaskQ;
            std::mutex _mTaskQ;

            // 모든 쓰레드 종료 flag
            bool _fStopAll;

            // Worker 쓰레드
            void WorkerThread();
        };
    };
};

#endif
