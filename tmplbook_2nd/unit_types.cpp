#include <cstddef>
#include <utility>
#include <type_traits>

template <std::size_t N, std::size_t D = 1>
struct Ratio {
    static constexpr std::size_t num = N;
    static constexpr std::size_t den = D;
    using type = Ratio<num, den>;
};

template <typename R1, typename R2>
struct RatioAddImp {
private:
    static constexpr std::size_t den = R1::den * R2::den;
    static constexpr std::size_t num = R1::num * R2::den + R2::num * R1::den;
public:
    using type = Ratio<num, den>;
};

template <typename R1, typename R2>
using RatioAdd = typename RatioAddImp<R1, R2>::type;

template <typename T, typename U = Ratio<1>>
class Duration {
public:
    using value_type = T;
    using unit_type = typename U::type;
private:
    value_type val;
public:
    constexpr Duration(): val{}{}
    constexpr Duration(value_type v): val(v){}
    constexpr value_type value() const { return val; }
};

template <typename T1, typename U1, typename T2, typename U2>
constexpr auto operator+(const Duration<T1, U1>& lhs, const Duration<T2, U2>& rhs) {
    using U = Ratio<1, RatioAdd<U1, U2>::den>;
    auto val = lhs.value() * U1::num * U2::den + rhs.value() * U2::num * U1::den;
    return Duration<decltype(val), U>(std::move(val));
}

int main(){
    constexpr auto d1 = Duration<int>(1); // 1 second
    constexpr auto d2 = Duration<int, Ratio<1, 1000>>(1); //1 milisecond
    constexpr auto d3 = d1 + d2;
    static_assert(d3.value() == 1001);
    static_assert(std::is_same_v<decltype(d3)::unit_type, Ratio<1, 1000>>);
    return 0;
}

