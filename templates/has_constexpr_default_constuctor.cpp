#include <iostream>
#include <tuple>

template <typename T>
constexpr bool test_fn(T) { return true; }

template <typename T>
struct has_constexpr_defualt_constructor_impl
{
  template <typename T1, bool = test_fn(T1{})>
  static std::true_type test(void*);
  template <typename T1>
  static std::false_type test(...);
  using type = decltype(test<T>(nullptr));
};

template <typename T>
struct has_constexpr_default_constructor: has_constexpr_defualt_constructor_impl<T>::type {};
template <typename T>
constexpr bool has_constexpr_default_constructor_v = has_constexpr_default_constructor<T>::value;

struct X
{
  int x;
  X() {}
};

struct Y
{
  int y = 0;
  constexpr Y() {}
};

struct Z
{
  X x;
};

template <typename T, T>
struct Test{};

int main()
{
  static_assert(has_constexpr_default_constructor_v<int>);
  static_assert(!has_constexpr_default_constructor_v<X>);
  static_assert(has_constexpr_default_constructor_v<Y>);
  static_assert(!has_constexpr_default_constructor_v<Z>);
  return 0;
}

