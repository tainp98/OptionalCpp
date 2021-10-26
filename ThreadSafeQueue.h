#ifndef THREADSAFEQUEUE_H
#define THREADSAFEQUEUE_H
#include <queue>
#include <mutex>
#include <thread>
template <typename T>
class ThreadSafeQueue{
public:
    ThreadSafeQueue(){}
    ThreadSafeQueue(const ThreadSafeQueue& other) = delete;
    ThreadSafeQueue& operator=(const ThreadSafeQueue& other) = delete ;

    void push(T data){
        std::lock_guard<std::mutex> lock(theMutex_);
        theQueue_.push(std::move(data));
    }

    bool empty(){
        std::lock_guard<std::mutex> lock(theMutex_);
        return theQueue_.empty();
    }

    bool tryPop(T& res){
        std::lock_guard<std::mutex> lock(theMutex_);
        if(theQueue_.empty()){
            return false;
        }
        res = std::move(theQueue_.front());
        theQueue_.pop();
        return true;
    }

    int size(){
        std::lock_guard<std::mutex> lock(theMutex_);
        return theQueue_.size();
    }
private:
    std::queue<T> theQueue_;
    mutable std::mutex theMutex_;
};
#endif // THREADSAFEQUEUE_H
