/*
g++ atoms.cpp -o atoms -lpthread
./atoms
*/

#include <atomic>
#include <iostream>
#include <thread>
#include <vector>


void print_vec(std::vector<int>* vec ){
    std::cout << std::endl;
    for (auto& el : *vec) std::cout << el << " ";
    std::cout << std::endl;
}

std::atomic<bool> busy (false);
std::atomic_int out (0);

void do_thing(std::vector<int>* inputs) {
    while(!busy) {
        std::this_thread::yield();
    }

    int next = inputs->back();
    inputs->pop_back();

    int out_ = out.load(std::memory_order_relaxed);
    out.store(out_ + next, std::memory_order_relaxed);
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
        busy = true;
        for (auto& th : threads) th.join();
        std::cout << out.load(std::memory_order_relaxed);
        std::cout << std::endl;
    }
    catch (const std::exception& ex) {
        std::cout << ex.what() << std::endl;
    }

    return 0;
}
