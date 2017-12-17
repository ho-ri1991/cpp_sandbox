#include <utility>
#include <type_traits>
#include <iterator>
#include <vector>
#include <list>
#include <iostream>

template <typename T>
struct identity {
    using type = T;
};

template <typename ...Types>
struct match_overloads: match_overloads<Types>... {
    using match_overloads<Types>::match...;
};


template <>
struct match_overloads<>{};

template <typename T>
struct match_overloads<T> {
    static identity<T> match(identity<T>);
};

template <typename T, typename... Types>
struct best_match_in_set {
    using type = typename decltype(match_overloads<Types...>::match(identity<T>{}))::type;
};

template <typename T, typename... Types>
using best_match_in_set_t = typename best_match_in_set<T, Types...>::type;

template <typename Iterator,
          typename IteratorTag = best_match_in_set_t<
	                             typename std::iterator_traits<Iterator>::iterator_category,
				     std::input_iterator_tag,
				     std::bidirectional_iterator_tag,
				     std::random_access_iterator_tag
				     >
				 >
struct X;

template <typename Iterator>
struct X<Iterator, std::input_iterator_tag>{
    void f(){std::cout<<"input_iterator"<<std::endl;}
};
;
template <typename Iterator>
struct X<Iterator, std::bidirectional_iterator_tag>{
    void f(){std::cout<<"bidirectional_iterator"<<std::endl;}
};

template <typename Iterator>
struct X<Iterator, std::random_access_iterator_tag>{
    void f(){std::cout<<"random_access_iterator"<<std::endl;}
};

int main(){
    static_assert(std::is_same_v<std::random_access_iterator_tag,
                                 best_match_in_set_t<
				     typename std::iterator_traits<std::vector<int>::iterator>::iterator_category,
				     std::input_iterator_tag,
				     std::bidirectional_iterator_tag,
				     std::random_access_iterator_tag
				 >>);
    static_assert(std::is_same_v<std::bidirectional_iterator_tag,
                                 best_match_in_set_t<
				     typename std::iterator_traits<std::list<int>::iterator>::iterator_category,
				     std::input_iterator_tag,
				     std::bidirectional_iterator_tag,
				     std::random_access_iterator_tag
				 >>);

    X<std::vector<int>::iterator>{}.f();
    X<std::list<int>::iterator>{}.f();
}

