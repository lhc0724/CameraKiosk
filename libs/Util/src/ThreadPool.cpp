#include "ThreadPool.h"

namespace CameraKiosk
{
    namespace Util
    {
        using namespace std;

        void ThreadPool::WorkerThread()
        {
            while (true)
            {
                unique_lock<mutex> lock(_mTaskQ);
                _cvTaskQ.wait(lock, [this]()
                              { return !this->_tasks.empty() || _fStopAll; });
                if (_fStopAll && this->_tasks.empty())
                {
                    return;
                }

                // 맨 앞의 task를 뺀다.
                function<void()> task = move(_tasks.front());
                _tasks.pop();
                lock.unlock();

                // 해당 task 수행
                task();
            }
        }

        template <class F, class... Args>
        future<typename invoke_result<F(Args...)>::type>
        ThreadPool::EnqueueTask(F &&f, Args &&...args)
        {
            if (_fStopAll)
            {
                throw runtime_error("ThreadPool stop all...");
            }

            using return_type = typename invoke_result<F(Args...)>::type;

            auto task = make_shared<packaged_task<return_type()>>(
                bind(forward<F>(f), forward<Args>(args)...));

            future<return_type> futureTaskRes = task->get_future();
            {
                lock_guard<mutex> lock(_mTaskQ);
                _tasks.push([task]()
                            { (*task)(); });
            }
            _cvTaskQ.notify_one();

            return futureTaskRes;
        }

    };
};