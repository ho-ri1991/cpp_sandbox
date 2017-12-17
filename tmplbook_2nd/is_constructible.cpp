#include <utility>
#include <type_traits>

namespace detail{
    template <typename T, typename Enabler, typename... Args>
    struct IsConstructibleImpl: std::false_type{};

    template <typename T, typename... Args>
    struct IsConstructibleImpl<T, std::void_t<decltype(T(std::declval<Args>()...))>, Args...>: std::true_type{};
}

template <typename T, typename... Args>
struct IsConstructible: detail::IsConstructibleImpl<T, void, Args...>{};

template <typename T, typename... Args>
constexpr bool IsConstructibleV = IsConstructible<T, Args...>::value;

struct X{
    explicit operator bool(){return true;}
    operator int(){return 0;}
    explicit operator std::false_type(){return std::false_type{};}
    X(int, bool){}
};

//another implementation of IsConstructible
namespace detail {
    template <typename T, typename... Args>
    struct IsConstructibleImpl2 {
    private:
//        template <typename T1, typename = std::void_t<decltype(T1(std::declval<Args>()...))>>
//        static std::true_type test(void*);
        template <typename T1, typename = decltype(T1(std::declval<Args>()...))>
        static std::true_type test(void*);
	template <typename>
        static std::false_type test(...);
    public:
	using type = decltype(test<T>(nullptr));
    };
}

template <typename T, typename... Args>
struct IsConstructible2: detail::IsConstructibleImpl2<T, Args...>::type{};

template <typename T, typename... Args>
constexpr bool IsConstructible2V = IsConstructible2<T, Args...>::value;

int main(){
    static_assert(IsConstructibleV<bool, X>);
    static_assert(IsConstructibleV<int, X>);
    static_assert(IsConstructibleV<X, int, bool>);
    static_assert(IsConstructibleV<double, X>);
    static_assert(IsConstructibleV<std::false_type, X>);
    static_assert(!IsConstructibleV<std::true_type, X>);
    static_assert(!IsConstructibleV<X, int, int, double>);

    static_assert(IsConstructible2V<bool, X>);
    static_assert(IsConstructible2V<int, X>);
    static_assert(IsConstructible2V<X, int, bool>);
    static_assert(IsConstructible2V<double, X>);
    static_assert(IsConstructible2V<std::false_type, X>);
    static_assert(!IsConstructible2V<std::true_type, X>);
    static_assert(!IsConstructible2V<X, int, int, double>);
   std::false_type f = static_cast<std::false_type>(X{1, false});
//   std::true_type t = static_cast<std::true_type>(X{1, true});
}

