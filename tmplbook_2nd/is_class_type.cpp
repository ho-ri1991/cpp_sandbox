#include <type_traits>

template <typename T, typename = void>
struct is_class_type: std::false_type{};

template <typename T>
struct is_class_type<T, std::void_t<int T::*>>: std::true_type{};

class X{};

int main(){
    static_assert(is_class_type<X>::value);
    static_assert(!is_class_type<int>::value);
}

