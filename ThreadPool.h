#ifndef THREADPOOL_H
#define THREADPOOL_H
#include "ThreadSafeQueue.h"
#include <queue>
#include <memory>
#include <future>
#include <iostream>
#include <functional>

class ThreadPool{
public:
    ThreadPool(unsigned int size = std::thread::hardware_concurrency())
        : done_(false)
    {
        workQueue_ = std::make_shared<ThreadSafeQueue<std::function<void()>>>();
        for(unsigned int i = 0; i < size; i++)
        {
            threads_.emplace_back([this](){ doTask(); });
        }
    }
    ~ThreadPool()
    {
        done_ = true;
        for(int i = 0; i < threads_.size(); i++){
            if(threads_[i].joinable())
                threads_[i].join();
        }
    }
    template<typename Function, typename... Args>
    void submit(Function&& f, Args&&... args){
        using ReturnType = typename std::result_of<Function(Args...)>::type;
        using FuncType = ReturnType(Args...);
        using Wrapped = std::packaged_task<FuncType>;
        std::shared_ptr<Wrapped> task = std::make_shared<Wrapped>(std::move(f));

        workQueue_->push([task, a = std::make_tuple(std::move(args)...)]() mutable {
            std::apply(*task, std::move(a));
        });

    }
private:
    void doTask()
    {
        while(!done_)
        {
        std::this_thread::sleep_for(std::chrono::seconds(1));
            std::function<void()> task;
            if(workQueue_->tryPop(task))
            {
                task();
            }
            else
            {
                printf("dont have task to do\n");
                std::this_thread::sleep_for(std::chrono::seconds(1));
                std::this_thread::yield();
            }
        }
    }
private:
    std::shared_ptr<ThreadSafeQueue<std::function<void()>>> workQueue_;
    std::atomic_bool done_;
    std::vector<std::thread> threads_;
};
#endif // THREADPOOL_H
