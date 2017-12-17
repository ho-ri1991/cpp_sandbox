#include <utility>
#include <type_traits>

template <typename From, typename To>
struct IsConvertibleImpl {
private:
    static void convert(To);
    template <typename T, typename = decltype(convert(std::declval<T>()))>
    static std::true_type test(void*);
    template <typename>
    static std::false_type test(...);
public:
    using type = decltype(test<From>(nullptr));
};

template <typename From, typename To>
struct IsConvertible: IsConvertibleImpl<From, To>::type{};

template <typename From, typename To>
constexpr bool IsConvertibleV = IsConvertible<From, To>::value;

//another implementation of IsConvertible
namespace detail{
    template <typename To>
    void test(To);
    
    template <typename From, typename To, typename = void>
    struct IsConvertibleImpl2: std::false_type{};
    
    template <typename From, typename To>
    struct IsConvertibleImpl2<From, To, std::void_t<decltype(test<To>(std::declval<From>()))>>: std::true_type{};
}

template <typename From, typename To>
struct IsConvertible2: detail::IsConvertibleImpl2<From, To>{};

template <typename From, typename To>
constexpr bool IsConvertible2V = IsConvertible2<From, To>::value;

struct X{
    operator bool() const {return false;};
    explicit operator std::false_type() const {return std::false_type{};}
};

void f(int){};

int main(){
    static_assert(IsConvertibleV<X, bool>);
    static_assert(IsConvertible2V<X, bool>);
    static_assert(!IsConvertibleV<X, std::false_type>);
    static_assert(!IsConvertible2V<X, std::false_type>);
}

