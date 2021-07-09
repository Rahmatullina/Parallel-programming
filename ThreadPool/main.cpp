#include <iostream>
#include<functional>
#include<thread>
#include <deque>
#include <mutex>
#include <optional>
#include "Task.h"
#include "THQueue.h"

std::mutex g_display_mutex;
#include "ThreadPool.h"

void AllThreadsWorkTest(){
    int num = 4;
    ThreadPool pool(num );
    if(pool.all_threads_work())
        std::cout << "ALL " << num << " THREADS WORK " <<  std::endl;
    pool.Dispose();
}
void AddOneTaskTest(){
    auto f = []
            {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "One task completed" << std::endl;
            };
    FunctionWrapper<void> funcWrapper(std::move(f));
    int num = 2;
    ThreadPool pool(num);
    pool.Enqueue<void>(std::make_unique<MyTask<void>>(std::move(funcWrapper)));
    pool.Dispose();
}
void AddManyTasksTest(){
    int num = 6;
    ThreadPool pool(num);
    for(int k = 0;k <10; k++) {
        auto f = [k] {
            std::this_thread::sleep_for(std::chrono::seconds(k % 2));
            std::lock_guard<std::mutex> lock(g_display_mutex);
            std::cout << k << " task completed" << std::endl;
        };
        FunctionWrapper<void> funcWrapper(std::move(f));
        pool.Enqueue<void>(std::make_unique<MyTask<void>>(std::move(funcWrapper)));
    }
    pool.Dispose();
}
void ContinueWithTest(){
    auto f1 = [] {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::lock_guard<std::mutex> lock(g_display_mutex);
        std::cout <<  "First task completed" << std::endl;
        return 44;
    };
    auto f2 = [] (int i){
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::lock_guard<std::mutex> lock(g_display_mutex);
        std::cout << "Obtained value from first task:"<< i << ". Second task completed" << std::endl;
    };

    FunctionWrapper<int> funcWrapper1(std::move(f1));
    FunctionWrapper<void, int> funcWrapper2(std::move(f2));

    int num = 6;
    ThreadPool pool(num);
    std::unique_ptr task1  = std::make_unique<MyTask<int>>(std::move(funcWrapper1));
    std::unique_ptr<MyTask<void>> task2 = task1->ContinueWith(std::move(funcWrapper2));

    pool.Enqueue<int>(std::move(task1));
    pool.Enqueue<void>(std::move(task2));
    pool.Dispose();
}
void CascadeContinueWithTest(){
    auto f1 = [] {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        std::lock_guard<std::mutex> lock(g_display_mutex);
        std::cout <<  "First task completed" << std::endl;
        return 44;
    };
    auto f2 = [] (int i){
        std::this_thread::sleep_for(std::chrono::seconds(4));
        std::lock_guard<std::mutex> lock(g_display_mutex);
        std::cout << "Obtained value from first task:"<< i << ". Second task completed" << std::endl;
        return "77";
    };
    auto f3 = [] (bool b){
        std::this_thread::sleep_for(std::chrono::seconds(2));
        std::lock_guard<std::mutex> lock(g_display_mutex);
        std::cout << "Obtained value from second task:"<< b << ". Third task completed" << std::endl;
        return 3.14;
    };
    auto f4 = [] (double d){
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::lock_guard<std::mutex> lock(g_display_mutex);
        std::cout << "Obtained value from third task:"<< d << ". Fourth task completed" << std::endl;
    };

    FunctionWrapper<int> funcWrapper1(std::move(f1));
    FunctionWrapper<bool, int> funcWrapper2(std::move(f2));
    FunctionWrapper<double, bool> funcWrapper3(std::move(f3));
    FunctionWrapper<void, double> funcWrapper4(std::move(f4));

    int num = 6;
    ThreadPool pool(num);
    std::unique_ptr task1  = std::make_unique<MyTask<int>>(std::move(funcWrapper1));
    std::unique_ptr<MyTask<bool>> task2 = task1->ContinueWith(std::move(funcWrapper2));
    std::unique_ptr<MyTask<double>> task3 = task2->ContinueWith(std::move(funcWrapper3));
    std::unique_ptr<MyTask<void>> task4 = task3->ContinueWith(std::move(funcWrapper4));

    pool.Enqueue<int>(std::move(task1));
    pool.Enqueue<bool>(std::move(task2));
    pool.Enqueue<double>(std::move(task3));
    pool.Enqueue<void>(std::move(task4));
    pool.Dispose();
}
void ErrorTest(){
    auto f = []
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "One task threw exception" << std::endl;
        throw std::invalid_argument("Some invalid argument exception.");
    };
    FunctionWrapper<void> funcWrapper(std::move(f));
    int num = 4;
    ThreadPool pool(num);
    pool.Enqueue<void>(std::make_unique<MyTask<void>>(std::move(funcWrapper)));
    pool.Dispose();
}
int main() {
    std::cout << "-------------All threads work Test" << "------------------------------------------------\n";
    AllThreadsWorkTest();
    std::cout << "-------------One task Test" << "------------------------------------------------\n";
    AddOneTaskTest();
    std::cout << "-------------More tasks than threads Test" << "------------------------------------------------\n";
    AddManyTasksTest();
    std::cout << "-------------ContinueWith Test" << "------------------------------------------------\n";
    ContinueWithTest();
    std::cout << "-------------Cascase ContinueWith Test" << "------------------------------------------------\n";
    CascadeContinueWithTest();
    std::cout << "-------------Error Test" << "------------------------------------------------\n";
    ErrorTest();
    std::cout << "-------------Tests end" << "------------------------------------------------\n";
    return 0;
}
