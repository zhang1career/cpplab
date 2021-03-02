//
// Created by 张荣晋 on 2021/3/2.
//

consteval int getValue(int input) {
    return input << 1;
}


int main() {
    const int val = getValue(123);
    return val;
}