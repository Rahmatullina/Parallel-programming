#include <iostream>
#include <queue>
#include <mutex>
#include <thread>
#include <optional>
#include <random>
#include "header.h"
#include <windows.h>

int main() {
    int c = 1; int p = 2;
    bool stop = false;
    bool testComleted;
    std::vector<Consumer<int>> consumers;
    std::vector<Producer<int>> producers;
    ThreadSafeQueue<int> Queue;
    TestStruct<int> test(0,0);

    for (int i = 0; i < p ; i++) {
        producers.emplace_back(std::ref(Queue), std::ref(test));
    }
    for (int i = 0; i < c ; i++) {
        consumers.emplace_back(std::ref(Queue), std::ref(test));
    }
    for (int i=0; i< p; i++)
    {
        producers[i].start();
    }
    for (int i = 0; i < c ; i++) {
        consumers[i].start();
    }
    while (!stop){
        if(GetKeyState(VK_MENU) & 0x8000) { //ALT key
            stop = true;
            for (int i = 0; i < p; i++) {
                producers[i].stop();
            }
            while(!Queue.empty()){}
            for (int i = 0; i < c; i++) {
                consumers[i].stop();
            }
            testComleted = test.test();
        }
    }
    if(testComleted)
        std::cout <<"Test Completed with no errors" << std::endl;
    else
        std::cout <<"Test Completed with ERRORS" << std::endl;
    std::cout <<"THE END" << std::endl;
}
