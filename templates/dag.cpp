#include <type_traits>
#include <tuple>
#include <iostream>
#include <boost/type_index.hpp>

namespace my
{
  template <typename T>
  struct identity
  {
    using type = T;
  };

  template <template <typename...> class Class, typename T>
  struct is_match_template: std::false_type{};

  template <template <typename...> class Class, typename ...Args>
  struct is_match_template<Class, Class<Args...>>: std::true_type{};

  template <template <typename...> class Class, typename T>
  constexpr bool is_match_template_v = is_match_template<Class, T>::value;

  template <typename... Ts>
  struct type_list {};
  
  namespace type_list_meta
  {
    template <typename TypeList>
    struct head;
    template <typename Head, typename... Tail>
    struct head<type_list<Head, Tail...>>
    {
      using type = Head;
    };
    template <typename...Ts>
    using head_t = typename head<Ts...>::type;
    template <typename TypeList>
    struct tail;
    template <typename Head, typename... Tail>
    struct tail<type_list<Head, Tail...>>
    {
      using type = type_list<Tail...>;
    };
    template <typename... Ts>
    using tail_t = typename tail<Ts...>::type;
    template <typename TypeList, typename...TypeLists>
    struct merge
    {
      using type = typename merge<TypeList, typename merge<TypeLists...>::type>::type;
    };
    template <typename...Ts1, typename... Ts2>
    struct merge<type_list<Ts1...>, type_list<Ts2...>>
    {
      using type = type_list<Ts1..., Ts2...>;
    };
    template <typename... Ts>
    struct merge<type_list<Ts...>>
    {
      using type = type_list<Ts...>;
    };
    template <typename... TypeLists>
    using merge_t = typename merge<TypeLists...>::type;
    template <typename TypeList>
    struct reverse
    {
      using type = merge_t<typename reverse<tail_t<TypeList>>::type, type_list<head_t<TypeList>>>;
    };
    template <>
    struct reverse<type_list<>>
    {
      using type = type_list<>;
    };
    template <typename TypeList>
    using reverse_t = typename reverse<TypeList>::type;
    template <typename TypeList>
    struct last
    {
      using type = head_t<reverse_t<TypeList>>;
    };
    template <>
    struct last<type_list<>> {};
    template <typename TypeList>
    using last_t = typename last<TypeList>::type;
  }

  struct terminal_node {};
  static constexpr auto terminal = terminal_node{};

  template <typename Next = terminal_node>
  struct inner_node
  {
    template <typename T>
    using this_template = inner_node<T>;
    template <typename NextNodeType>
    this_template<NextNodeType> copy() const { return this_template<NextNodeType>{}; }
    Next* next = nullptr;
    void construct_connection() {}
  };

  template <>
  struct inner_node<terminal_node>
  {
    template <typename T>
    using this_template = inner_node<T>;
    template <typename T>
    this_template<T> copy() const { return this_template<T>{}; }
    const terminal_node* next = &terminal;
    void construct_connection() {}
  };

  template <typename N1, typename N2, typename... Ns>
  struct chained_node
  {
    using head_type = N1;
    using last_type = typename std::conditional_t<sizeof...(Ns) == 0, identity<N2>, type_list_meta::last<type_list<Ns...>>>::type;
    static constexpr auto chain_size = sizeof...(Ns) + 2;
    std::tuple<N1, N2, Ns...> ns;
    constexpr chained_node() = default;
    constexpr chained_node(const std::tuple<N1, N2, Ns...>& ns): ns(ns) {}
    constexpr chained_node(std::tuple<N1, N2, Ns...>&& ns): ns(std::move(ns)) {}
    chained_node(const chained_node&) = default;
    chained_node(chained_node&&) = default;
    ~chained_node() = default;
    template <typename NextNodeType, typename Node1, typename Node2, typename... Nodes>
    static auto merge(Node1&& node1, Node2&& node2, Nodes&&... nodes)
    {
      if constexpr (sizeof...(Nodes) == 0)
        return merge<NextNodeType>(std::forward<Node1>(node1), std::forward<Node2>(node2));
      else
        return merge<NextNodeType>(std::forward<Node1>(node1), merge<NextNodeType>(std::forward<Node2>(node2), std::forward<Nodes>(nodes)...));
    }
    template <typename NextNodeType, typename Node1, typename Node2>
    static auto merge(Node1&& node1, Node2&& node2)
    {
      using Node2Type = std::decay_t<Node2>;
      if constexpr (is_match_template_v<std::tuple, Node2Type>)
      {
        using Node1NextNodeType = std::tuple_element_t<0, Node2Type>;
        return std::tuple_cat(std::make_tuple(node1.template copy<Node1NextNodeType>()), std::forward<Node2>(node2));
      }
      else
      {
        using Node1NextNodeType = decltype(node2.template copy<NextNodeType>());
        return std::make_tuple(node1.template copy<Node1NextNodeType>(), node2.template copy<NextNodeType>());
      }
    }
    template <typename... Nodes>
    static auto make(const std::tuple<Nodes...>& nodes) { return chained_node<Nodes...>(nodes); }
    template <typename... Nodes>
    static auto make(std::tuple<Nodes...>&& nodes) { return chained_node<Nodes...>(std::move(nodes)); }
    template <typename NextNodeType, std::size_t... Is>
    auto copy_impl(std::index_sequence<Is...>) const { return merge<NextNodeType>(std::get<Is>(ns)...); }
    template <typename NextNodeType>
    auto copy() const { return make(copy_impl<NextNodeType>(std::make_index_sequence<sizeof...(Ns) + 2>{})); }
    template <std::size_t I>
    void construct_connection_impl();
    void construct_connection();
  };

  template <typename N1, typename N2, typename... Ns>
  struct or_node
  {
    using content_types = type_list<N1, N2, Ns...>;
    std::tuple<N1, N2, Ns...> ns;
    constexpr or_node() = default;
    constexpr or_node(const std::tuple<N1, N2, Ns...>& ns): ns(ns) {}
    constexpr or_node(std::tuple<N1, N2, Ns...>&& ns): ns(std::move(ns)) {}
    or_node(const or_node&) = default;
    or_node(or_node&&) = default;
    ~or_node() = default;
    template <typename... Nodes>
    static auto make(const std::tuple<Nodes...>& nodes) { return or_node<Nodes...>(nodes); }
    template <typename... Nodes>
    static auto make(std::tuple<Nodes...>&& nodes) { return or_node<Nodes...>(std::move(nodes)); }
    template <typename NextNodeType, std::size_t... Is>
    auto copy_impl(std::index_sequence<Is...>) const { return std::make_tuple(std::get<Is>(ns).template copy<NextNodeType>()...); }
    template <typename NextNodeType>
    auto copy() const { return make(copy_impl<NextNodeType>(std::make_index_sequence<sizeof...(Ns) + 2>{})); }
    template <std::size_t I>
    void construct_connection_impl();
    void construct_connection();
  };

  template <typename Node>
  struct node_heads
  {
    using type = type_list<Node>;
    static auto get_head_pointers(Node& node) { return std::make_tuple(&node); }
  };
  template <typename N1, typename... Ns>
  struct node_heads<chained_node<N1, Ns...>>
  {
    using type = typename node_heads<N1>::type;
    static auto get_head_pointers(chained_node<N1, Ns...>& node) { return node_heads<N1>::get_head_pointers(std::get<0>(node.ns)); }
  };
  template <typename N1, typename... Ns>
  struct node_heads<or_node<N1, Ns...>>
  {
    using type = type_list_meta::merge_t<typename node_heads<N1>::type, typename node_heads<Ns>::type...>;
    template <std::size_t... Is>
    static auto get_head_pointers_impl(or_node<N1, Ns...>& node, std::index_sequence<Is...>)
    {
      return std::tuple_cat(node_heads<Ns>::get_head_pointers(std::get<Is + 1>(node.ns))...);
    }
    static auto get_head_pointers(or_node<N1, Ns...>& node)
    {
      return std::tuple_cat(
        node_heads<N1>::get_head_pointers(std::get<0>(node.ns)),
        get_head_pointers_impl(node, std::make_index_sequence<sizeof...(Ns)>{})
      );
    }
  };

  template <typename Node>
  struct node_lasts
  {
    using type = type_list<Node>;
    static auto get_last_pointers(Node& node) { return std::make_tuple(&node); }
  };
  template <typename... Ns>
  struct node_lasts<chained_node<Ns...>>
  {
    using node_type = chained_node<Ns...>;
    using this_last = typename node_type::last_type;
    using type = typename node_lasts<this_last>::type;
    static auto get_last_pointers(node_type& node)
    {
      return node_lasts<this_last>::get_last_pointers(std::get<sizeof...(Ns) - 1>(node.ns));
    }
  };
  template <typename... Ns>
  struct node_lasts<or_node<Ns...>>
  {
    using node_type = or_node<Ns...>;
    using type = type_list_meta::merge_t<typename node_lasts<Ns>::type...>;
    template <std::size_t... Is>
    static auto get_last_pointers_impl(node_type& node, std::index_sequence<Is...>)
    {
      return std::tuple_cat(node_lasts<Ns>::get_last_pointers(std::get<Is>(node.ns))...);
    }
    static auto get_last_pointers(node_type& node)
    {
      return get_last_pointers_impl(node, std::make_index_sequence<sizeof...(Ns)>{});
    }
  };

  template <typename Tuple, std::size_t... Is>
  auto tail_impl(Tuple&& t, std::index_sequence<Is...>)
  {
    return std::make_tuple(std::get<Is + 1>(std::forward<Tuple>(t))...);
  }

  template <typename Tuple>
  auto tail(Tuple&& t)
  {
    return tail_impl(std::forward<Tuple>(t), std::make_index_sequence<std::tuple_size_v<Tuple> - 1>{});
  }

  template <typename T, typename... Ts>
  auto remove_last(const std::tuple<T, Ts...>& t)
  {
    if constexpr (sizeof...(Ts) == 0)
      return std::tuple<>{};
    else
      return std::tuple_cat(std::tuple<T>{std::get<0>(t)}, remove_last(tail(t)));
  }

//  template <typename N1, typename N2>
//  auto operator+(N1&& n1, N2&& n2)
//  {
//    using Node1 = std::decay_t<N1>;
//    using Node2 = std::decay_t<N2>;
//    if constexpr (
//      is_match_template_v<chained_node, Node1> &&
//      is_match_template_v<chained_node, Node2>
//    )
//    { // both nodes are chained node
//      if constexpr (
//        is_match_template_v<or_node, typename Node1::last_type> &&
//        is_match_template_v<or_node, typename Node2::head_type>
//      )
//      { // joint points are both or_node
//      }
//      else if constexpr (
//        is_match_template_v<or_node, typename Node1::las_typet> &&
//        !is_match_template_v<or_node, typename Node2::head_type>
//      )
//      { // joint point of Node1 is or_node, that of Node2 is not
//      }
//      else if constexpr (
//        !is_match_template_v<or_node, typename Node1::last_type> &&
//        is_match_template_v<or_node, typename Node2::head_type>
//      )
//      { // joint point of Node2 is or_node, that of Node1 is not
//      }
//      else
//      {
//        auto ans = std::tuple_cat(
//          remove_last(n1),
//          std::make_tuple(std::get<Node1::chain_size - 1>(n1).copy<typename Node2::head_type>()),
//          n2
//        );
//        // update pointer
//      }
//    }
//    else if constexpr (
//  }
//  template <typename N1, typename N2>
//  auto operator|(N1&& n1, N2&& n2)
//  {
//    using Node1 = std::decay_t<N1>;
//    using Node2 = std::decay_t<N2>;
//  }
}

int main ()
{
  using namespace my;
  using namespace my::type_list_meta;
  static_assert(std::is_same_v<head_t<type_list<int, bool, double>>, int>);
  static_assert(std::is_same_v<tail_t<type_list<int, bool, double>>, type_list<bool, double>>);
  static_assert(std::is_same_v<reverse_t<type_list<int, bool, double>>, type_list<double, bool, int>>);
  static_assert(std::is_same_v<last_t<type_list<int, bool, double>>, double>);
  static_assert(std::is_same_v<merge_t<type_list<int, bool>, type_list<double, float>, type_list<>>, type_list<int, bool, double, float>>);
  using example_node1 =
    chained_node<
      or_node<
        chained_node<inner_node<inner_node<inner_node<inner_node<>>>>, inner_node<inner_node<inner_node<>>>, inner_node<inner_node<>>>,
        chained_node<inner_node<inner_node<inner_node<>>>, inner_node<inner_node<>>>,
        inner_node<inner_node<>>
      >,
      inner_node<>
    >;
  static_assert(
    std::is_same_v<
      typename node_heads<example_node1>::type,
      type_list<inner_node<inner_node<inner_node<inner_node<>>>>, inner_node<inner_node<inner_node<>>>, inner_node<inner_node<>>>
    >
  );
  auto example1 = example_node1{};
  static_assert(
    std::is_same_v<
      decltype(
        node_heads<example_node1>::get_head_pointers(example1)
      ),
      std::tuple<inner_node<inner_node<inner_node<inner_node<>>>>*, inner_node<inner_node<inner_node<>>>*, inner_node<inner_node<>>*>
    >
  );
  static_assert(
    std::is_same_v<
      decltype(example1.copy<inner_node<>>()),
      chained_node<
        or_node<
          chained_node<inner_node<inner_node<inner_node<inner_node<inner_node<>>>>>, inner_node<inner_node<inner_node<inner_node<>>>>, inner_node<inner_node<inner_node<>>>>,
          chained_node<inner_node<inner_node<inner_node<inner_node<>>>>, inner_node<inner_node<inner_node<>>>>,
          inner_node<inner_node<inner_node<>>>
        >,
        inner_node<inner_node<>>
      >
    >
  );

  static_assert(
    std::is_same_v<
      typename node_lasts<
        chained_node<
          inner_node<>,
          or_node<
            chained_node<inner_node<inner_node<inner_node<>>>, inner_node<inner_node<>>, inner_node<>>,
            chained_node<inner_node<inner_node<>>, inner_node<>>,
            inner_node<>
          >
        >
      >::type,
      type_list<inner_node<>, inner_node<>, inner_node<>>
    >
  );
  
  using example_node2 =
    chained_node<
      inner_node<>,
      or_node<
        chained_node<inner_node<inner_node<inner_node<>>>, inner_node<inner_node<>>, inner_node<>>,
        chained_node<inner_node<inner_node<>>, inner_node<>>,
        inner_node<>
      >
    >;
  auto example2 = example_node2{};
  static_assert(
    std::is_same_v<
      decltype(
        node_lasts<example_node2>::get_last_pointers(example2)
      ),
      std::tuple<inner_node<>*, inner_node<>*, inner_node<>*>
    >
  );
  return 0;
}
