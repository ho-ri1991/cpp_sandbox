#include <type_traits>

//This definition issues error because std::make_unsigned<T>::type is not defined for bool or non integral types.
//std::conditional instantiates both true/false path. Therefore, we need to utilize lazy instantiation.
//template <typename T>
//struct unsigned_type {
//    using type = std::conditional<std::is_integral_v<T> && !std::is_same_v<T, bool>,
//                                  typename std::make_unsigned_t<T>, T>;
//};

template <typename T>
struct identity {
    using type = T;
};

template <typename T>
struct make_unsigned_type {
    using type = std::make_unsigned_t<T>;
};

//In the case where the instance is not need to be a complete type, the compiler is permitted to instantiate only the declaration.
template <typename T>
struct unsigned_type {
    using type = typename std::conditional<std::is_integral_v<T> && !std::is_same_v<T, bool>,
                                           make_unsigned_type<T>, identity<T>>::type;
};


int main(){
    typename unsigned_type<int>::type i;
    typename unsigned_type<bool>::type j;
    return 0;
}

