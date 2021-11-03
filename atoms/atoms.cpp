#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

std::atomic<bool> ready (false);
std::atomic_flag winner = ATOMIC_FLAG_INIT;

/*
void print_vec(std::vector<int>* vec ){
    std::cout << std::endl;
    for (auto& el : *vec) std::cout << el << " ";
    std::cout << std::endl;
}
void mutate_vec(std::vector<int>* vec) {
    for(auto& el : *vec) el++;
}
int pop(std::vector<int>* vec) {
    int p = vec->back();
    vec->pop_back();
    return p;
}
*/

void count1m(int id) {
    while(!ready) {std::this_thread::yield();}
    for (volatile int i = 0; i < 1000000; i++){}
    if (!winner.test_and_set()) {std::cout << "thread #" << id << " won!" << std::endl;}
}

int main(void) {
    /*
    std::vector<int> vec;
    for(int i = 0; i < 128; i++) vec.push_back(i);
    print_vec(&vec);
    mutate_vec(&vec);
    std::cout << std::endl << pop(&vec) << std::endl;
    print_vec(&vec);
    */

    std::vector<std::thread> threads;
    std::cout << "10 thread genesis ..." << std::endl;
    for(int i = 0; i < 10; i++) {
        threads.push_back(std::thread(count1m, i));
    }
    ready = true;
    for (auto& th : threads) th.join();
    return 0;
}
