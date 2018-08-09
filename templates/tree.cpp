#include <iostream>
namespace my
{
  template <typename, typename>
  class node_base;
  template <typename R, typename... Args, typename T>
  class node_base<R(Args...), T>
  {
    friend T;
  private:
    using this_type = node_base<R(Args...), T>;
    using derived_type = T;
  protected:
    R** ans_ptr{};
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
  public:
    using custom_node_type = custom_node_base;
  public:
    custom_node_base() = default;
    custom_node_base(const custom_node_base&) = default;
    custom_node_base(custom_node_base&&) = default;
    custom_node_base& operator=(const custom_node_base&) = default;
    custom_node_base& operator=(custom_node_base&&) = default;
  public:
    const char* match(const char* str)
    {
      if (base_type::ans_ptr) *base_type::ans_ptr = nullptr;
      auto& d = static_cast<derived_type&>(*this);
      auto ans = d.N.match(str);
      if (ans)
      {
        d.as();
        if (base_type::ans_ptr) *base_type::ans_ptr = static_cast<R*>(this);
      }
      return ans;
    }
  };
  template <typename N1, typename N2>
  class multi_node: node_base<single_node_data(), multi_node<N1, N2>>
  {
  private:
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
      auto res1 = n1.match(begin);
      return res1 ? res1 : n2.match(begin);
    }
  };
  template <typename N1, typename N2>
  class chain_node: node_base<single_node(), chain_node<N1, N2>>
  {
  private:
    N1 n1;
    N2 n2;
  public:
    chain_node(const N1& n1, const N2& n2): n1(n1), n2(n2) {}
    chain_node(const chain_node&) = default;
    chain_node(chain_node&&) = default;
    chain_node& operator=(const chain_node&) = default;
    chain_node& operator=(chain_node&&) = default;
    ~chain_node() = default;
  public:
    const char* match(const char* begin)
    {
      auto res1 = n1.match(begin);
      if (!res1) return nullptr;
      return n2.match(res1);
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
    return chain_node(n1, n2);
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
    struct Clause0: custom_node_base<BLOCK1_data(), Clause0>
    {
      single_node_data* p1{};
      DEFINE_EXPRESSION("aaa"_s[p1], Clause0);
      void as()
      {
        if (p1) {
          x = -42;
        }
        else x = -1;
      }
    };
  };
  auto BLOCK1() { return BLOCK1_DEF::Clause0{}; }
  struct BLOCK2_DEF
  {
    struct BLOCK2_data
    {
      int x;
    };
    struct Clause0: custom_node_base<BLOCK2_data(), Clause0>
    {
      BLOCK1_DEF::BLOCK1_data* p1{};
//      single_node_data* p1{};
      single_node_data* p2{};
      DEFINE_EXPRESSION(BLOCK1()[p1] | "bbb"_s[p2], Clause0);
      void as()
      {
        if (p1) {
//          std::cout << p1->x << std::endl;
          x = p1->x;
        }
        else x = -1;
      }
    };
  };
  auto BLOCK2() { return BLOCK2_DEF::Clause0{}; }
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
    auto n = my::BLOCK1()[p];
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
  return 0;
}
