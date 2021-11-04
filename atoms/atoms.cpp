/*
g++ atoms.cpp -o atoms -lpthread
./atoms
*/

#include <atomic>
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>

std::mutex mtx;

std::atomic_int out (0);

void do_thing(std::vector<int>* inputs) {
    mtx.lock();
    int next = inputs->back();
    inputs->pop_back();
    mtx.unlock();
    out.store(out.load(std::memory_order_seq_cst) + next, std::memory_order_seq_cst);
}

int main(void) {

    std::vector<int> inputs;
    for(int i = 0; i < 100; i++) inputs.push_back(rand() % 10);

    // base
    int k = 0;
    for(auto& el : inputs) k += el;
    std::cout << k << std::endl;

    // threaded
    try {
        std::vector<std::thread> threads;
        for(int i = 0; i < 100; i++) threads.push_back(std::thread(do_thing, &inputs));
        for (auto& th : threads) th.join();
        std::cout << out.load(std::memory_order_seq_cst);
        std::cout << std::endl;
    }
    catch (const std::exception& ex) {
        std::cout << ex.what() << std::endl;
    }

    return 0;
}
