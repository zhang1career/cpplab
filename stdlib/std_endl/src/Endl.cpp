//
// Created by zhangrj on 2021/3/11.
//

#include <iostream>
#include <fstream>
#include <chrono>

void writeln(std::ostream& os, const std::string& str) {
    os << str << '\n';
}

void writeln_with_flush(std::ostream& os, const std::string& str) {
    os << str << '\n' << std::flush;
}

void writeln_with_endl(std::ostream& os, const std::string& str) {
    os << str << std::endl;
}

int main() {
    std::ofstream outfile("output.txt", std::ios_base::trunc);

    auto then = std::chrono::steady_clock::now();
    for (int i = 0; i <1000000; i++) {
        writeln(outfile, "Hello World");
    }
    outfile << std::flush;
    auto now = std::chrono::steady_clock::now();
    auto diff = now - then;
    std::cout << "writeln: " << std::chrono::duration <double, std::milli> (diff).count() << " ms" << std::endl;

    then = std::chrono::steady_clock::now();
    for (int i = 0; i <1000000; i++) {
        writeln_with_flush(outfile, "Hello World");
    }
    now = std::chrono::steady_clock::now();
    diff = now - then;
    std::cout << "flush: " << std::chrono::duration <double, std::milli> (diff).count() << " ms" << std::endl;

    then = std::chrono::steady_clock::now();
    for (int i = 0; i <1000000; i++) {
        writeln_with_endl(outfile, "Hello World");
    }
    now = std::chrono::steady_clock::now();
    diff = now - then;
    std::cout << "endl: " << std::chrono::duration <double, std::milli> (diff).count() << " ms" << std::endl;
}
