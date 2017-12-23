#include <cstddef>
#include <type_traits>

template <int... Ns>
struct Indexes{};

template <int... Ns1, int... Ns2>
constexpr Indexes<Ns1..., Ns2...> index_cat(Indexes<Ns1...>, Indexes<Ns2...>){ return Indexes<Ns1..., Ns2...>{}; }

template <int first, int last, typename Fn>
constexpr auto range(Fn fn) {
    static_assert(first <= last);
    if constexpr (first == last) {
        return Indexes<fn(first)>{};
    } else {
        return index_cat(Indexes<fn(first)>{}, range<first + 1, last>(fn));
    }
}

template <typename Base, int D>
struct Discriminator: public Base{};

template <typename... Types>
struct TypeList;

template <typename, typename>
struct PolicySelectorImpl;

template <typename... Policies, int... Ns>
struct PolicySelectorImpl<TypeList<Policies...>, Indexes<Ns...>>: Discriminator<Policies, Ns>... {};

//helper class for avoiding lambda in unevaluated context
template <int N>
struct PolicyIndexHelper {
    static constexpr auto indexes = range<1, N>([](auto x){return x;});
};

template <typename... Policies>
struct PolicySelector: PolicySelectorImpl<TypeList<Policies...>, std::remove_const_t<decltype(PolicyIndexHelper<sizeof...(Policies)>::indexes)>> {};


struct DefaultPolicy1{};
struct DefaultPolicy2{};
struct DefaultPolicy3{};
struct DefaultPolicy4{};

struct DefaultPolicies {
    using P1 = DefaultPolicy1;
    using P2 = DefaultPolicy2;
    using P3 = DefaultPolicy3;
    using P4 = DefaultPolicy4;
};

struct DefaultPolicyArgs: virtual DefaultPolicies {};
template <typename Policy>
struct Policy1_is: virtual DefaultPolicies{
    using P1 = Policy;
};
template <typename Policy>
struct Policy2_is: virtual DefaultPolicies{
    using P2 = Policy;
};
template <typename Policy>
struct Policy3_is: virtual DefaultPolicies{
    using P3 = Policy;
};
template <typename Policy>
struct Policy4_is: virtual DefaultPolicies{
    using P4 = Policy;
};

template <typename PolicySetter1 = DefaultPolicyArgs,
          typename PolicySetter2 = DefaultPolicyArgs,
          typename PolicySetter3 = DefaultPolicyArgs,
          typename PolicySetter4 = DefaultPolicyArgs>
struct BreadSlicer{
    using Policies = PolicySelector<PolicySetter1, PolicySetter2, PolicySetter3, PolicySetter4>;
};

struct CustomPolicy{};

template<auto t>
struct X{};

int main(){
    using b = BreadSlicer<Policy3_is<CustomPolicy>>;
    static_assert(std::is_same_v<DefaultPolicy1, b::Policies::P1>);
    static_assert(std::is_same_v<DefaultPolicy2, b::Policies::P2>);
    static_assert(std::is_same_v<CustomPolicy, b::Policies::P3>);
    static_assert(std::is_same_v<DefaultPolicy4, b::Policies::P4>);
    return 0;
}

