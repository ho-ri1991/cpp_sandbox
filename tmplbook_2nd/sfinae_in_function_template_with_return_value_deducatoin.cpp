#include <iostream>

// signiture of function templates includes template parameter list
template <typename T, decltype(T::value)* = nullptr>
auto func(T) {
    std::cout << "value: " << T::value << std::endl;
}

template <typename T, typename = typename T::value_type, void* = nullptr>
auto func(T) {
    std::cout << "value_type: " << typeid(typename T::value_type).name() << std::endl;
}

struct has_value {
    static constexpr int value = 1;
};

struct has_value_type {
    using value_type = int;
};

int main() {
    func(has_value{}); // value
    func(has_value_type{}); // value_type
    return 0;
}

