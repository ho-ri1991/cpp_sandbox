#include <type_traits>
#include <tuple>
#include <iostream>
#include <boost/type_index.hpp>
#include <cassert>

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

    template <typename>
    struct to_tuple;
    template <typename... Ts>
    struct to_tuple<type_list<Ts...>>
    {
      using type = std::tuple<Ts...>;
    };
    template <typename TypeList>
    using to_tuple_t = typename to_tuple<TypeList>::type;
  }

  template <typename>
  struct inner_node;
  template <typename, typename, typename...>
  struct chained_node;
  template <typename, typename, typename...>
  struct or_node;

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
  using node_heads_t = typename node_heads<Node>::type;

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
  template <typename Node>
  using node_lasts_t = typename node_lasts<Node>::type;

  struct terminal_node {};
  static constexpr auto terminal = terminal_node{};

  template <typename NextNodeType>
  struct next_node
  {
    using next_hold_type = std::add_pointer_t<NextNodeType>;
    next_hold_type next{};
  };
  template <typename... Ts>
  struct next_node<std::tuple<Ts...>>
  {
    using next_hold_type = std::tuple<std::add_pointer_t<Ts>...>;
    next_hold_type next{};
  };
  template <>
  struct next_node<terminal_node>
  {
    using next_hold_type = const terminal_node*;
    next_hold_type next = &terminal;
  };

  template <typename Next = terminal_node>
  struct inner_node: next_node<Next>
  {
    template <typename T>
    using this_template = inner_node<T>;
    template <typename NextNodeType>
    constexpr this_template<NextNodeType> copy() const { return this_template<NextNodeType>{phrase}; }
    void construct_connection() {}
    const char* phrase = "<null>";
    inner_node() = default;
    constexpr inner_node(const char* phrase): next_node<Next>(), phrase(phrase) {}
    inner_node(const inner_node&) = default;
    inner_node(inner_node&&) = default;
    ~inner_node() = default;
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
    constexpr chained_node(const chained_node&) = default;
    constexpr chained_node(chained_node&&) = default;
    ~chained_node() = default;
    template <typename NextNodeType, typename Node1, typename Node2, typename... Nodes>
    static constexpr auto merge(Node1&& node1, Node2&& node2, Nodes&&... nodes)
    {
      if constexpr (sizeof...(Nodes) == 0)
        return merge<NextNodeType>(std::forward<Node1>(node1), std::forward<Node2>(node2));
      else
        return merge<NextNodeType>(std::forward<Node1>(node1), merge<NextNodeType>(std::forward<Node2>(node2), std::forward<Nodes>(nodes)...));
    }
    template <typename NextNodeType, typename Node1, typename Node2>
    static constexpr auto merge(Node1&& node1, Node2&& node2)
    {
      using Node2Type = std::decay_t<Node2>;
      if constexpr (is_match_template_v<std::tuple, Node2Type>)
      {
        using Node1NextNodeTypeTuple = type_list_meta::to_tuple_t<node_heads_t<std::tuple_element_t<0, Node2Type>>>;
        using Node1NextNodeType = std::conditional_t<std::tuple_size_v<Node1NextNodeTypeTuple> == 1, std::tuple_element_t<0, Node1NextNodeTypeTuple>, Node1NextNodeTypeTuple>;
        return std::tuple_cat(std::make_tuple(node1.template copy<Node1NextNodeType>()), std::forward<Node2>(node2));
      }
      else
      {
        using Node1NextNodeTypeTuple = type_list_meta::to_tuple_t<node_heads_t<decltype(node2.template copy<NextNodeType>())>>;
        using Node1NextNodeType = std::conditional_t<std::tuple_size_v<Node1NextNodeTypeTuple> == 1, std::tuple_element_t<0, Node1NextNodeTypeTuple>, Node1NextNodeTypeTuple>;
        return std::make_tuple(node1.template copy<Node1NextNodeType>(), node2.template copy<NextNodeType>());
      }
    }
    template <typename... Nodes>
    static constexpr auto make(const std::tuple<Nodes...>& nodes) { return chained_node<Nodes...>(nodes); }
    template <typename... Nodes>
    static constexpr auto make(std::tuple<Nodes...>&& nodes) { return chained_node<Nodes...>(std::move(nodes)); }
    template <typename NextNodeType, std::size_t... Is>
    constexpr auto copy_impl(std::index_sequence<Is...>) const { return merge<NextNodeType>(std::get<Is>(ns)...); }
    template <typename NextNodeType>
    constexpr auto copy() const { return make(copy_impl<NextNodeType>(std::make_index_sequence<sizeof...(Ns) + 2>{})); }
    template <std::size_t I, typename Tuple1, typename Tuple2>
    static void set_next_node_helper(Tuple1&& t1, Tuple2&& t2)
    {
      if constexpr (I < std::tuple_size_v<Tuple1>)
      {
        if constexpr (std::tuple_size_v<std::decay_t<Tuple2>> == 1)
          std::get<I>(t1)->next = std::get<0>(t2);
        else
          std::get<I>(t1)->next = t2;
        set_next_node_helper<I + 1>(std::forward<Tuple1>(t1), std::forward<Tuple2>(t2));
      }
    }
    template <std::size_t I>
    void construct_connection_impl()
    {
      std::get<I>(ns).construct_connection();
      if constexpr (I < sizeof...(Ns) + 1)
      {
        set_next_node_helper<0>(node_lasts<std::tuple_element_t<I, decltype(ns)>>::get_last_pointers(std::get<I>(ns)), node_heads<std::tuple_element_t<I + 1, decltype(ns)>>::get_head_pointers(std::get<I + 1>(ns)));
        construct_connection_impl<I + 1>();
      }
    }
    void construct_connection() { construct_connection_impl<0>(); }
  };

  template <typename N1, typename N2, typename... Ns>
  struct or_node
  {
    using content_types = type_list<N1, N2, Ns...>;
    std::tuple<N1, N2, Ns...> ns;
    constexpr or_node() = default;
    constexpr or_node(const std::tuple<N1, N2, Ns...>& ns): ns(ns) {}
    constexpr or_node(std::tuple<N1, N2, Ns...>&& ns): ns(std::move(ns)) {}
    constexpr or_node(const or_node&) = default;
    constexpr or_node(or_node&&) = default;
    ~or_node() = default;
    template <typename... Nodes>
    static constexpr auto make(const std::tuple<Nodes...>& nodes) { return or_node<Nodes...>(nodes); }
    template <typename... Nodes>
    static constexpr auto make(std::tuple<Nodes...>&& nodes) { return or_node<Nodes...>(std::move(nodes)); }
    template <typename NextNodeType, std::size_t... Is>
    constexpr auto copy_impl(std::index_sequence<Is...>) const { return std::make_tuple(std::get<Is>(ns).template copy<NextNodeType>()...); }
    template <typename NextNodeType>
    constexpr auto copy() const { return make(copy_impl<NextNodeType>(std::make_index_sequence<sizeof...(Ns) + 2>{})); }
    template <std::size_t I>
    void construct_connection_impl()
    {
      if constexpr (I == sizeof...(Ns) + 2)
        return;
      else
      {
        std::get<I>(ns).construct_connection();
        construct_connection_impl<I + 1>();
        return;
      }
    }
    void construct_connection() { construct_connection_impl<0>(); }
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

  template <typename N1, typename N2>
  constexpr auto operator+(N1&& n1, N2&& n2)
  {
    using Node1 = std::decay_t<N1>;
    using Node2 = std::decay_t<N2>;
    using Node2Heads = type_list_meta::to_tuple_t<node_heads_t<Node2>>;
//    static constexpr auto Node2HeadSize = std::tuple_size_v<Node2Heads>;
    using Node1NewNextNodes = std::conditional_t<std::tuple_size_v<Node2Heads> == 1, std::tuple_element_t<0, Node2Heads>, Node2Heads>;
    auto head_nodes = n1.template copy<Node1NewNextNodes>();
    if constexpr (
      is_match_template_v<chained_node, Node1> &&
      is_match_template_v<chained_node, Node2>
    )
    { // both nodes are chained node
      return chained_node(std::tuple_cat(std::move(head_nodes).ns, std::forward<N2>(n2).ns));
    }
    else if constexpr (
      is_match_template_v<chained_node, Node1> &&
      !is_match_template_v<chained_node, Node2>
    )
    {
      return chained_node(std::tuple_cat(std::move(head_nodes).ns, std::make_tuple(std::forward<N2>(n2))));
    }
    else if constexpr (
      !is_match_template_v<chained_node, Node1> &&
      is_match_template_v<chained_node, Node2>
    )
    {
      return chained_node(std::tuple_cat(std::make_tuple(std::move(head_nodes)), std::forward<N2>(n2).ns));
    }
    else
    {
      return chained_node(std::make_tuple(std::move(head_nodes), std::forward<N2>(n2)));
    }
  }
  template <typename N1, typename N2>
  constexpr auto operator|(N1&& n1, N2&& n2)
  {
    using Node1 = std::decay_t<N1>;
    using Node2 = std::decay_t<N2>;
    if constexpr (
      is_match_template_v<or_node, Node1> &&
      is_match_template_v<or_node, Node2>
    )
    {
      return or_node(std::tuple_cat(std::forward<N1>(n1).ns, std::forward<N2>(n2).ns));
    }
    else if constexpr (
      is_match_template_v<or_node, Node1> &&
      !is_match_template_v<or_node, Node2>
    )
    {
      return or_node(std::tuple_cat(std::forward<N1>(n1).ns, std::tuple(std::forward<N2>(n2))));
    }
    else if constexpr (
      !is_match_template_v<or_node, Node1> &&
      is_match_template_v<or_node, Node2>
    )
    {
      return or_node(std::tuple_cat(std::tuple(std::forward<N1>(n1)), std::forward<N2>(n2).ns));
    }
    else
    {
      return or_node(std::tuple(std::forward<N1>(n1), std::forward<N2>(n2)));
    }
  }
}

template <typename Node>
void dfs(Node* node);
template <std::size_t I = 0, typename... Ts>
void dfs(std::tuple<Ts...> nodes);
void dfs(const my::terminal_node*) {}

template <typename Node>
void dfs(Node* node)
{
  assert(node);
  std::cout << node->phrase << std::endl;
  if constexpr (my::is_match_template_v<std::tuple, typename Node::next_hold_type>)
    dfs(node->next);
  else
    dfs(node->next);
}


template <std::size_t I = 0, typename... Ts>
void dfs(std::tuple<Ts...> nodes)
{
  if constexpr (I < sizeof...(Ts))
  {
    assert(std::get<I>(nodes));
    std::cout << std::get<I>(nodes)->phrase << std::endl;
    dfs(std::get<I>(nodes)->next);
    dfs<I + 1>(nodes);
  }
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

  using example_node2 =
    chained_node<
      inner_node<>,
      or_node<
        chained_node<inner_node<inner_node<inner_node<>>>, inner_node<inner_node<>>, inner_node<>>,
        chained_node<inner_node<inner_node<>>, inner_node<>>,
        inner_node<>
      >
    >;
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
  
  auto example2 = example_node2{};
  static_assert(
    std::is_same_v<
      decltype(
        node_lasts<example_node2>::get_last_pointers(example2)
      ),
      std::tuple<inner_node<>*, inner_node<>*, inner_node<>*>
    >
  );

  constexpr auto c1 = inner_node<>{"a"} + (inner_node<>{"b"} + inner_node<>{"c"});
  constexpr auto c2 = (inner_node<>{"f"} + inner_node<>{"g"}) + inner_node<>{"h"};
  constexpr auto c3 = (inner_node<>{"i"} + inner_node<>{"j"}) + (inner_node<>{"k"} | inner_node<>{"l"});

//  c1.construct_connection();
//  dfs(node_heads<decltype(c1)>::get_head_pointers(c1));
  std::cout << "===" << std::endl;

  std::cout << boost::typeindex::type_id_with_cvr<decltype(c1)>().pretty_name() << std::endl;
  std::cout << boost::typeindex::type_id_with_cvr<decltype(c2)>().pretty_name() << std::endl;
  std::cout << boost::typeindex::type_id_with_cvr<decltype(c3)>().pretty_name() << std::endl;

  auto or1 = c1 + (inner_node<>{"d"} | c3 | inner_node{"e"}) + c2;
  std::cout << boost::typeindex::type_id_with_cvr<decltype(or1)>().pretty_name() << std::endl;

  or1.construct_connection();
  std::cout << "==" << std::endl;
  dfs(node_heads<decltype(or1)>::get_head_pointers(or1));

  constexpr auto or2 = c1 + (inner_node<>{"d"} | c3 | inner_node{"e"}) + c2;

  return 0;
}
