#include <iostream>
#include <boost/asio.hpp>
#include <thread>
#include <fstream>

// TODO: codestyle
// TODO: logging

int main() {
    // open file

    std::thread t1([](){
        std::fstream file("test");
        file.seekg(12);
        file << "Hello world!";
    });
    std::thread t2([](){
        std::fstream file("test");
        file << "Hello world!";
    });
    t1.join();
    t2.join();
}
