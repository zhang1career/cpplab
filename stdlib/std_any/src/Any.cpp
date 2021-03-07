//
// Created by 张荣晋 on 2021/3/4.
//
#include <any>
#include <vector>
#include <string>
#include <iostream>
#include <type_traits>

struct S {
    S(const S& s) = default;
    S() = default;
};

int main() {
    std::vector<std::any> v{5, 3.4, std::string("Hello, world!"), S()};
    static_assert(std::is_nothrow_move_constructible<S>::value, "no-throw");

    std::cout << sizeof(std::any) << '\n';
    std::cout << v.size() << '\n';
    std::cout << v[1].type().name() << '\n';

    int* i = std::any_cast<int>(&v[0]);
    *i = 10;
    std::cout << std::any_cast<int>(v[0]) << '\n';
}
