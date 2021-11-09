#include <iostream>
#include <vector>
#include <fstream>

int main(void) {

    // write data from file into local vector
    std::vector<char> vec;

    std::ifstream file;

    file.exceptions(
        std::ifstream::badbit
        | std::ifstream::failbit
        | std::ifstream::eofbit);

    file.open("text.txt", std::ifstream::in | std::ifstream::binary);
    file.seekg(0, std::ios::end);
    std::streampos length(file.tellg());
    if (length) {
        file.seekg(0, std::ios::beg);
        vec.resize(static_cast<std::size_t>(length));
        file.read(&vec.front(), static_cast<std::size_t>(length));
    }

    // check output (size in bytes, elements in vector)
    std::cout << length << std::endl;
    std::cout << std::endl;
    for(auto& el : vec) {
        std::cout << el;
    } std::cout << std::endl;
    return 0;
}
