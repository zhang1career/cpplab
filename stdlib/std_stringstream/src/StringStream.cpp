//
// Created by zhangrj on 2021/3/11.
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>

void write_file(std::ostream& os) {
    os << "do something\n";
    os << "do something else\n";
}

int main() {
    std::ofstream outfile("output.txt", std::ios_base::trunc);

    auto then = std::chrono::steady_clock::now();
    write_file(outfile);
    outfile << std::flush;
    auto now = std::chrono::steady_clock::now();
    auto diff = now - then;
    std::cout << "write_file: " << std::chrono::duration <double, std::milli> (diff).count() << " ms" << std::endl;

    then = std::chrono::steady_clock::now();
    std::stringstream ss;
    write_file(ss);
    outfile << ss.str();
    now = std::chrono::steady_clock::now();
    diff = now - then;
    std::cout << "stringstream: " << std::chrono::duration <double, std::milli> (diff).count() << " ms" << std::endl;
}
