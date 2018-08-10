#include <iostream>
#include <tuple>
#include <variant>
#include <cassert>
namespace my
{
  template <typename T>
  struct placeholer
  {
    std::variant<placeholer<T>*, T> data;
    T get()
    {
      return std::visit([](auto&& var) {
          using U = std::decay_t<decltype(var)>;
          if constexpr (std::is_same_v<U, placeholer<T>*>)
          {
            assert(var);
            return var->get();
          }
          else
             return var;
        }, data
      );
    }
    void set(T d) { data = d; }
    void set(placeholer<T>& p) { data = &p; }
    placeholer() = default;
    placeholer(T t): data(std::move(t)) {}
    placeholer(placeholer<T>& p): data(&p){}
    placeholer(const placeholer&) = default;
    placeholer(placeholer&&) = default;
    placeholer& operator=(const placeholer&) = default;
    placeholer& operator=(placeholer&&) = default;
    ~placeholer() = default;
  };

  template <typename F, typename Tuple, std::size_t... Is>
  auto placeholder_apply_impl(F&& f, Tuple&& placeholder_pack, std::index_sequence<Is...>)
  {
    return f(std::get<Is>(placeholder_pack).get()...);
  }
  template <typename F, typename Tuple>
  auto placeholder_apply(F&& f, Tuple&& placeholder_pack)
  {
    return placeholder_apply_impl(std::forward<F>(f), std::forward<Tuple>(placeholder_pack), std::make_integer_sequence<std::size_t, std::tuple_size_v<std::decay_t<Tuple>>>{});
  }

  template <typename, typename>
  class node_base;
  template <typename R, typename... Args, typename T>
  class node_base<R(Args...), T>
  {
    friend T;
  private:
    using this_type = node_base<R(Args...), T>;
    using derived_type = T;
  public:
    using result_type = R;
  protected:
    result_type** ans_ptr{};
    double weight = 1.0;
    ~node_base() = default;
  public:
    node_base() = default;
    node_base(const node_base&) = default;
    node_base(node_base&&) = default;
    node_base& operator=(const node_base&) = default;
    node_base& operator=(node_base&&) = default;
  public:
    derived_type operator[](R*& ptr) const
    {
      auto tmp = static_cast<const derived_type&>(*this);
      tmp.ans_ptr = &ptr;
      return tmp;
    }
  };
  struct single_node_data {};
  class single_node: public node_base<single_node_data(), single_node>
  {
  private:
    using base_type = node_base<single_node_data(), single_node>;
    const char* phrase;
    single_node_data data;
  public:
    constexpr single_node(const char* str): phrase((str && str[0] == '\0') ? nullptr : str) {}
    single_node(const single_node&) = default;
    single_node(single_node&&) = default;
    single_node& operator=(const single_node&) = default;
    single_node& operator=(single_node&&) = default;
    ~single_node() = default;
    const char* match(const char* begin)
    {
      if (base_type::ans_ptr)
        *base_type::ans_ptr = nullptr;
      if (!phrase) return begin;
      else
      {
        const char* p = phrase, *b = begin;
        while (*p != '\0' && *b != '\0')
        {
          if (*p != *b) return nullptr;
          ++p;
          ++b;
        }
        if (*p != '\0') return nullptr;
        else if (*b != ' ' && *b != '\0') return nullptr;
        if (base_type::ans_ptr)
          *base_type::ans_ptr = &data;
        return *b == ' ' ? b + 1 : b;
      }
    }
  };
  template <typename, typename>
  class custom_node_base;
  template <typename R, typename... Args, typename T>
  class custom_node_base<R(Args...), T>: public node_base<R(Args...), T>, public R
  {
    friend T;
  private:
    using base_type = node_base<R(Args...), T>;
    using derived_type = T;
    ~custom_node_base() = default;
    std::tuple<placeholer<Args>...> placeholder_pack;
  public:
    template <typename... Ts>
    static derived_type generate(Ts&&... ts)
    {
      auto tmp = derived_type{};
      tmp.placeholder_pack = decltype(placeholder_pack)(ts...);
      return tmp;
    }
    using custom_node_type = custom_node_base;
    using node_invoke_type = R(Args...);
  public:
    custom_node_base() = default;
    custom_node_base(const custom_node_base&) = default;
    custom_node_base(custom_node_base&&) = default;
    custom_node_base& operator=(const custom_node_base&) = default;
    custom_node_base& operator=(custom_node_base&&) = default;
  public:
    template <std::size_t I>
    auto& arg() { return std::get<I>(placeholder_pack); }
    const char* match(const char* str)
    {
      if (base_type::ans_ptr) *base_type::ans_ptr = nullptr;
      auto& d = static_cast<derived_type&>(*this);
      auto ans = d.N.match(str);
      if (ans)
      {
        placeholder_apply([&d](auto&&... args){ return d.as(args...); }, placeholder_pack);
        if (base_type::ans_ptr) *base_type::ans_ptr = static_cast<R*>(this);
      }
      return ans;
    }
  };
  template <typename N1, typename N2>
  class multi_node: public node_base<single_node_data(), multi_node<N1, N2>>
  {
  private:
    using base_type = node_base<single_node_data(), multi_node<N1, N2>>;
    using result_type  = typename base_type::result_type;
    result_type data;
    N1 n1;
    N2 n2;
  public:
    multi_node(const N1& n1, const N2& n2): n1(n1), n2(n2) {}
    multi_node(const multi_node&) = default;
    multi_node(multi_node&&) = default;
    multi_node& operator=(const multi_node&) = default;
    multi_node& operator=(multi_node&&) = default;
    ~multi_node() = default;
  public:
    const char* match(const char* begin)
    {
      if (base_type::ans_ptr)
        *base_type::ans_ptr = nullptr;
      auto res1 = n1.match(begin);
      if (res1)
      {
        if (base_type::ans_ptr)
          *base_type::ans_ptr = &data;
        return res1;
      }
      auto res2 = n2.match(begin);
      if (res2)
      {
        if (base_type::ans_ptr)
          *base_type::ans_ptr = &data;
        return res2;
      }
      return nullptr;
    }
  };
  template <typename N1, typename N2>
  class chain_node: public node_base<single_node_data(), chain_node<N1, N2>>
  {
  private:
    using base_type = node_base<single_node_data(), chain_node<N1, N2>>;
    using result_type  = typename base_type::result_type;
    result_type data;
    N1 n1;
    N2 n2;
  public:
    chain_node(const N1& n1, const N2& n2): n1(n1), n2(n2) {}
    chain_node(const chain_node&) = default;
    chain_node(chain_node&&) = default;
    chain_node& operator=(const chain_node&) = delete;
    chain_node& operator=(chain_node&&) = delete;
    ~chain_node() = default;
  public:
    const char* match(const char* begin)
    {
      if (base_type::ans_ptr)
        *base_type::ans_ptr = nullptr;
      auto res1 = n1.match(begin);
      if (!res1) return nullptr;
      auto res2 = n2.match(res1);
      if (!res2) return nullptr;
      if (base_type::ans_ptr)
        *base_type::ans_ptr = &data;
      return res2;
    }
  };
  // TODO write constraints for SFINAE
  template <typename N1, typename N2>
  auto operator|(const N1& n1, const N2& n2)
  {
    return multi_node<N1, N2>(n1, n2);
  }
  // TODO write constraints for SFINAE
  template <typename N1, typename N2>
  auto operator+(const N1& n1, const N2& n2)
  {
    return chain_node<N1, N2>(n1, n2);
  }
}
my::single_node operator"" _s(const char* str, std::size_t len) noexcept { return my::single_node(str); }
#define DEFINE_EXPRESSION(EXPR, CLAUSE_CLASS) \
public: \
  decltype(EXPR) N = EXPR; \
  CLAUSE_CLASS() = default; \
  CLAUSE_CLASS(const CLAUSE_CLASS& other):custom_node_type(other), N(EXPR) {} \
  CLAUSE_CLASS(CLAUSE_CLASS&& other):custom_node_type(std::move(other)), N(EXPR) {} \
  CLAUSE_CLASS& operator=(const CLAUSE_CLASS&) = delete; \
  CLAUSE_CLASS& operator=(CLAUSE_CLASS&&) = delete; \
  ~CLAUSE_CLASS() = default;

namespace my
{
  struct BLOCK1_DEF
  {
    struct BLOCK1_data
    {
      int x;
    };
    struct Clause0: custom_node_base<BLOCK1_data(int), Clause0>
    {
      single_node_data* p1{};
      DEFINE_EXPRESSION("aaa"_s[p1], Clause0);
      void as(int y)
      {
        if (p1) {
          x = y;
        }
        else x = -1;
      }
    };
  };
  template <typename T>
  auto BLOCK1(T&& t) { return BLOCK1_DEF::Clause0::generate(std::forward<T>(t)); }
  struct BLOCK2_DEF
  {
    struct BLOCK2_data
    {
      int x;
    };
    struct Clause0: custom_node_base<BLOCK2_data(), Clause0>
    {
      BLOCK1_DEF::BLOCK1_data* p1{};
      single_node_data* p2{};
      DEFINE_EXPRESSION(BLOCK1(42)[p1] | "bbb"_s[p2], Clause0);
      void as()
      {
        if (p1) {
//          std::cout << p1->x << std::endl;
          x = p1->x * 2;
        }
        else x = -1;
      }
    };
  };
  auto BLOCK2() { return BLOCK2_DEF::Clause0{}; }
  struct BLOCK3_DEF
  {
    struct BLOCK3_data
    {
      int x;
    };
    struct Clause0: custom_node_base<BLOCK3_data(), Clause0>
    {
      BLOCK2_DEF::BLOCK2_data* p1{};
      single_node_data* p2{};
      single_node_data* p3{};
      DEFINE_EXPRESSION((BLOCK2()[p1] + "bbb"_s[p2] + "dd"_s)[p3] | "ccc"_s, Clause0);
      void as()
      {
        if (p1 && p2 && p3) {
//          std::cout << p1->x << std::endl;
          x = p1->x;
        }
        else x = -1;
      }
    };
  };
  auto BLOCK3() { return BLOCK3_DEF::Clause0{}; }
}
int main()
{
  {
    my::single_node_data* p = nullptr;
    auto n = "aaa"_s[p];
    std::cout << "ans: " << (p == nullptr) << std::endl;
    auto input = "aaa";
    auto output = n.match(input);
    std::cout << output << std::endl;
    std::cout << "ans: " << (p == nullptr) << std::endl;
  }
//  {
//    my::single_node_data* p = nullptr;
//    auto n = "aaa"_s[p];
//    std::cout << "ans: " << (p == nullptr) << std::endl;
//    auto input = "aaaa bbb";
//    auto output = n.match(input);
//    std::cout << (void*)output << std::endl;
//    std::cout << "ans: " << (p == nullptr) << std::endl;
//  }
//  {
//    auto n = "aaa"_s;
//    auto input = "aaa bbb";
//    auto output = n.match(input);
//    std::cout << output << std::endl;
//  }
  {
    std::cout << "===" << std::endl;
    my::BLOCK1_DEF::BLOCK1_data* p = nullptr;
    auto n = my::BLOCK1(-42)[p];
//    std::cout << "ans: " << (p == nullptr) << std::endl;
    auto input = "aaa bbb";
    auto output = n.match(input);
    std::cout << output << std::endl;
//    std::cout << p << std::endl;
    std::cout << "ans: " << p->x << std::endl;
  }
  {
    std::cout << "===" << std::endl;
    my::BLOCK2_DEF::BLOCK2_data* p = nullptr;
    auto n = my::BLOCK2()[p];
    {
      auto input = "aaa bbb";
      auto output = n.match(input);
      std::cout << output << std::endl;
      std::cout << "ans: " << p->x << std::endl;
    }
    {
      p = nullptr;
      auto input = "bbb";
      auto output = n.match(input);
      std::cout << output << std::endl;
      std::cout << "ans: " << p->x << std::endl;
    }
    {
      p = nullptr;
      auto input = "bbbc";
      auto output = n.match(input);
      std::cout << (output == nullptr) << std::endl;
    }
  }
  {
    std::cout << "===" << std::endl;
    my::BLOCK3_DEF::BLOCK3_data* p = nullptr;
    auto n = my::BLOCK3()[p];
    {
      auto input = "aaa bbb dd ccc";
      auto output = n.match(input);
      std::cout << output << std::endl;
      std::cout << "ans: " << p->x << std::endl;
    }
    {
      p = nullptr;
      auto input = "ccc";
      auto output = n.match(input);
      std::cout << output << std::endl;
      std::cout << "ans: " << p->x << std::endl;
    }
    {
      p = nullptr;
      auto input = "bbbc";
      auto output = n.match(input);
      std::cout << (output == nullptr) << std::endl;
    }
  }
  return 0;
}
