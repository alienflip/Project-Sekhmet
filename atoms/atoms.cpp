#include <atomic>
#include <iostream>
#include <thread>
#include <vector>
<<<<<<< HEAD
=======
#include <cassert>
>>>>>>> f4dd0100ca80a08ac56f20808de3bd38136f7b62

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

int main(void) {
    std::vector<int> vec;
    for(int i = 0; i < 128; i++) vec.push_back(i);
    print_vec(&vec);
    mutate_vec(&vec);
    std::cout << std::endl << pop(&vec) << std::endl;
    print_vec(&vec);
    return 0;
}