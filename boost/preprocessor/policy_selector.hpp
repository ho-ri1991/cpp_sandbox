#if !BOOST_PP_IS_ITERATING

#  ifndef POLICY_SELECTOR_INC
#    define POLICY_SELECTOR_INC

#    ifndef POLICY_SELECTOR_NUM
#      define POLICY_SELECTOR_NUM 5
#    endif // ifndef POLICY_SELECTOR_NUM

#    include <cstddef>
#    include <type_traits>
#    include <utility>
#    include <boost/preprocessor/cat.hpp>
#    include <boost/preprocessor/arithmetic/sub.hpp>
#    include <boost/preprocessor/arithmetic/add.hpp>
#    include <boost/preprocessor/iteration/iterate.hpp>
#    include <boost/preprocessor/iteration/local.hpp>
#    include <boost/preprocessor/repetition/enum_shifted_params.hpp>

template <typename Base, int D>
struct Discriminator: public Base{};

template <typename... Types>
struct TypeList;

template <typename, typename>
struct PolicySelectorImpl;

template <typename... Policies, int... Ns>
struct PolicySelectorImpl<TypeList<Policies...>, std::integer_sequence<int, Ns...>>: Discriminator<Policies, Ns>... {};

template <typename... Policies>
struct PolicySelector: PolicySelectorImpl<TypeList<Policies...>, std::make_integer_sequence<int, sizeof...(Policies)>> {};

#    define BOOST_PP_ITERATION_PARAMS_1 (3, (1, POLICY_SELECTOR_NUM, "policy_selector.hpp"))
#    include BOOST_PP_ITERATE()

#  endif // #ifndef POLICY_SELECTOR_INC
#elif BOOST_PP_ITERATION_DEPTH() == 1 // #if !BOOST_PP_IS_ITERATING

#  define n BOOST_PP_ITERATION()

template <typename Policy, typename DefaultPolicies>
struct BOOST_PP_CAT(Policy, BOOST_PP_CAT(n, _is)): virtual DefaultPolicies {
    using BOOST_PP_CAT(P, n) = Policy;
};

template <BOOST_PP_ENUM_SHIFTED_PARAMS(BOOST_PP_ADD(n, 1), typename DefaultPolicy)>
struct BOOST_PP_CAT(DefaultPolicies, n) {
#  define BOOST_PP_ITERATION_PARAMS_2 (3, (1, n , "policy_selector.hpp"))
#  include BOOST_PP_ITERATE()
};

#  undef n

#elif BOOST_PP_ITERATION_DEPTH() == 2 // #if !BOOST_PP_IS_ITERATING

#  define n BOOST_PP_ITERATION()

using BOOST_PP_CAT(P, n) = BOOST_PP_CAT(DefaultPolicy, n);

# undef n

#endif // #if !BOOST_PP_IS_ITERATING

