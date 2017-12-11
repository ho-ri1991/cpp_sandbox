#include <utility>
#include <vector>
#include <type_traits>

template <typename F, typename... Args, typename = decltype(std::declval<F>()(std::declval<Args&&>()...))>
std::true_type is_valid_impl(void*);

template <typename F, typename... Args>
std::false_type is_valid_impl(...);

inline constexpr auto is_valid = [](auto f){
    return [](auto&&... args){
        return decltype(is_valid_impl<decltype(f), decltype(args)&&...>(nullptr)){};
    };
};

template <typename T>
struct TypeT {
    using Type = T;
};

template <typename T>
constexpr auto type = TypeT<T>{};

template <typename T>
T valueT(TypeT<T>);

inline constexpr auto has_first_impl = is_valid([](auto x) -> decltype((void)valueT(x).first){});
template <typename T>
using has_first = decltype(has_first_impl(type<T>));
template <typename T>
constexpr bool has_first_v = has_first<T>::value;

inline constexpr auto has_size_type_impl = is_valid([](auto x) -> typename std::decay_t<decltype(valueT(x))>::size_type{});
template <typename T>
using has_size_type = decltype(has_size_type_impl(type<T>));
template <typename T>
constexpr bool has_size_type_v = has_size_type<T>::value;

int main(){
    static_assert(has_first_v<std::pair<int, double>>);
    static_assert(!has_first_v<int>);
    static_assert(has_size_type_v<std::vector<int>>);
    static_assert(!has_size_type_v<int>);
}

