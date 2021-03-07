//
// Created by 张荣晋 on 2021/3/4.
//
#include <iostream>
#include <sstream>
#include <vector>


template<typename... Param>
std::vector<std::string> to_string(const Param &...param) {
    const auto to_string_impl = [](const auto &p) {
        std::stringstream ss;
        ss << p;
        return ss.str();
    };
    return {to_string_impl(param)...};
}

int main() {
    const auto vec = to_string("hello", 1, 5.3, "world");

    for (const auto &v : vec) {
        std::cout << v << '\n';
    }
}
