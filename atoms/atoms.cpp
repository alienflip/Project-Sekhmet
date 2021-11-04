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

void do_thing(std::vector<int>* inputs, int* out) {
    while(!busy) {
        std::this_thread::yield();
    }

    int next = inputs->back();
    inputs->pop_back();

    (*out)+=next;
}

int main(void) {

    std::vector<int> inputs;
    for(int i = 0; i < 100; i++) inputs.push_back(rand() % 10);

    // base
    int k = 0;
    for(auto& el : inputs) k += el;
    std::cout << k << std::endl;

    int out = 0;
    
    // threaded
    try {
        std::vector<std::thread> threads;
        for(int i = 0; i < 10; i++) threads.push_back(std::thread(do_thing, &inputs, &out));
        busy = true;
        for (auto& th : threads) th.join();
    }
    catch (const std::exception& ex) {
        std::cout << ex.what() << std::endl;
    }

    std::cout << out << std::endl;

    return 0;
}
