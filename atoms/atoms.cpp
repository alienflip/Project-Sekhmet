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
/*
void mutate_vec(std::vector<int>* vec) {
    for(auto& el : *vec) el++;
}
int pop(std::vector<int>* vec) {
    int p = vec->back();
    vec->pop_back();
    return p;
}
*/


/*
std::atomic<bool> ready (false);
std::atomic_flag winner = ATOMIC_FLAG_INIT;

void count1m(int id) {
    while(!ready) {std::this_thread::yield();}
    for (volatile int i = 0; i < 1000000; i++){}
    if (!winner.test_and_set()) {std::cout << "thread #" << id << " won!" << std::endl;}
}
*/

/*
std::atomic<int> foo (0);

void set_foo(int x) {
    foo.store(x, std::memory_order_relaxed);
}

void print_foo() {
    int x;
    do {
        x = foo.load(std::memory_order_relaxed);
    } while (x == 0);
    std::cout << "foo: " << x << std::endl;
}
*/

/*
std::atomic<int> foo (0);

void set_foo(int x) {
    foo.store(x, std::memory_order_relaxed);
}

void print_foo() {
    int x;
    do {
        x = foo.load(std::memory_order_relaxed);
    } while(x == 0);
    std::cout << "foo: " << x << std::endl;
}
*/

std::atomic<bool> busy (false);

void do_thing(std::vector<int>* inputs, int* out) {
    while(busy) {std::this_thread::yield();}

    int next = inputs->back();
    inputs->pop_back();

    out->(+=next);
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

    /*
    std::vector<std::thread> threads;
    std::cout << "10 thread genesis ..." << std::endl;
    for(int i = 0; i < 10; i++) threads.push_back(std::thread(count1m, i));
    ready = true;
    for (auto& th : threads) th.join();
    */

    /*
    std::thread first (print_foo);
    std::thread second (set_foo, 10);
    first.join();
    second.join();
    */

    /*
    std::thread first (print_foo);
    std::thread second (set_foo, 10);
    first.join();
    second.join();
    */

    std::vector<int> inputs;
    for(int i = 0; i < 100; i++) inputs.push_back(rand() % 10);

    // base
    int k = 0;
    for(auto& el : inputs) k += el;
    std::cout << k << std::endl;
    print_vec(&inputs);

    // threaded
    std::vector<std::thread> threads;
    int out = 0;
    //for(int i = 0; i < 10; i++) threads.push_back(std::thread(do_thing, i));


    return 0;
}
