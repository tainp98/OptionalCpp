#include <iostream>
#include <optional>
#include <functional>
#include <memory>
#include <future>
#include <tuple>
#include <utility>
#include <ThreadPool.h>
using namespace std;
void function3(std::unique_ptr<int> arg1){
    std::cout << "arg1 = " << *arg1 << "\n";
}
//void function1(std::optional<std::unique_ptr<int>> arg1){
//    std::cout << "arg1 = " << **arg1 << "\n";
//}
void function2(int arg1){
    std::cout << "arg1 = " << arg1 << "\n";
}

template<typename T>
struct Shim{
    T get(){
        return std::move(*t_);
    }
    template<class... Args>
    Shim(Args&&... args) : t_(std::in_place,
                              std::forward<Args>(args)...){}
    Shim(Shim&&) = default;
    Shim& operator=(Shim&&) = default;
    Shim(const Shim&) {
        std::cout << "oops\n";
    }
    Shim& operator=(const Shim&){
        std::cout << "oops\n";
    }
private:
    std::optional<T> t_;
};
template <typename F,typename... Args>
void customFunction(F&& f, Args&&... args){
    using  resultType = typename std::result_of<F(Args...)>::type;
    std::shared_ptr<std::packaged_task<resultType(Args...)>> task = std::make_shared<std::packaged_task<resultType(Args...)>>(std::move(f));
    std::function<void()> func = [task, a = std::make_tuple(std::move(args)...)]() mutable {
        std::apply(*task, std::move(a));
    };
    func();
}

class Test1{
public:
    Test1(){

    }
    void run(std::unique_ptr<int> arg1){
        std::lock_guard<std::mutex> locker(mux_);
        std::cout << val << " " << *arg1 << "\n";
        val++;
        std::cout << val << " " << *arg1 << "\n";
    }
    void run1(int arg1){
        std::cout << val << " " << arg1 << "\n";
        val++;
        std::cout << val << " " << arg1 << "\n";
    }
    void run2(Shim<std::unique_ptr<int>>&& arg1){
        std::lock_guard<std::mutex> locker(mux_);
        std::cout << "run2 begin\n";
        std::unique_ptr<int> data = arg1.get();
        std::cout << val << " " << *data << "\n";
        val++;
        std::cout << val << "\n";
    }
private:
    int val{0};
    std::mutex mux_;
};

class Test2{
public:
    Test2(std::shared_ptr<Test1> test1, ThreadPool* threadPool)
        : test1_(test1)
    {
        threadPool_ = threadPool;
    }

    void run(){
        while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        threadPool_->submit(&Test1::run2, test1_.get(), Shim<std::unique_ptr<int>>(std::make_unique<int>(20)));
        threadPool_->submit(&Test1::run1, test1_.get(), 10);
        }
    }
private:
    std::shared_ptr<Test1> test1_;
    ThreadPool* threadPool_;
};

int main()
{
    std::shared_ptr<Test1> test = std::make_shared<Test1>();
//    customFunction(&Test1::run2, &test, Shim<std::unique_ptr<int>>(std::make_unique<int>(20)));
    ThreadPool* threadPool = new ThreadPool(5);
    threadPool->submit(&Test1::run2, test.get(), Shim<std::unique_ptr<int>>(std::make_unique<int>(20)));
    threadPool->submit(&Test1::run1, test.get(), 10);
    Test2 test2(test, threadPool);
    std::thread t = std::thread(&Test2::run, &test2);
    t.join();
    return 0;
}
