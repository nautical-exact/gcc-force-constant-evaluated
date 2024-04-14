#include <stdio.h>

constexpr void check_f(int x) {
    if (x % 2 == 0) {
        throw "Error: x is even";
    }
}

__attribute__((force_constant_evaluated))
constexpr int f(int x) {
    if (__builtin_is_constant_evaluated()) {
        check_f(x);
    }
    return x;
}

constexpr int g(int x) {
    if (__builtin_is_constant_evaluated()) {
        check_f(x);
    }
    return x;
}

int main() {
    int i = f(14);
    
    printf("%d\n", i);
}