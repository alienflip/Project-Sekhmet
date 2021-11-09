#include <iostream>
#include <vector>
#include <fstream>

const std::string path = "text.txt";

int main(void) {
    std::vector<char> vec;
    std::ifstream file;
    file.exceptions(
        std::ifstream::badbit
        | std::ifstream::failbit
        | std::ifstream::eofbit);
    try {
        file.open(path, std::ifstream::in | std::ifstream::binary);
        file.seekg(0, std::ios::end);
        std::streampos length(file.tellg());
        if (length) {
            file.seekg(0, std::ios::beg);
            vec.resize(static_cast<std::size_t>(length));
            file.read(&vec.front(), static_cast<std::size_t>(length));
        }
        std::cout << length << std::endl;
        std::cout << std::endl;
        for(auto& el : vec) {
            std::cout << el;
        } std::cout << std::endl;
    }
    catch (std::ifstream::failure e) {
        std::cerr << "Exception opening/reading/closing file" << std::endl;
    }

    return 0;
}
