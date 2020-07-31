#ifndef SPLITORDERED_HASHMAP_H
#define SPLITORDERED_HASHMAP_H

#include <list>
#include <vector>

namespace My
{
namespace SplitOrderedHashMap
{

template <typename Key, typename T, typename Hash = std::hash<Key>, typename Pred = std::equal_to<Key> /*TODO: Allocator*/>
class SplitOrderedHashMap
{
private:
  using ThisType = SplitOrderedHashMap;
  using HashValueType = unsigned long long;
  struct ListElementType
  {
  public:
    using ValueType = std::pair<const Key, T>;
  private:
    alignas(ValueType) std::byte mData[sizeof(ValueType)];
    HashValueType mKey;
  public:
    explicit ListElementType(const ValueType& value, HashValueType key);
    explicit ListElementType(ValueType&& value, HashValueType key) noexcept(std::is_nothrow_move_constructible_v<ValueType>);
    explicit ListElementType(HashValueType key) noexcept;
    ListElementType(const ListElementType&);
    ListElementType(ListElementType&&) noexcept(std::is_nothrow_move_constructible_v<ValueType>);
    ListElementType& operator=(const ListElementType&) = delete;
    ListElementType& operator=(ListElementType&&) = delete;
    ~ListElementType() noexcept;
    ValueType& getValue() noexcept;
    const ValueType& getValue() const noexcept;
    HashValueType getKey() const noexcept;
    bool isSentinel() const noexcept;
  };
  using ListType = std::list<ListElementType>;
  class Iterator
  {
    friend bool operator==(const Iterator& x, const Iterator& y)
    {
      return x.mListIterator == y.mListIterator;
    }
    friend bool operator!=(const Iterator& x, const Iterator& y)
    {
      return !(x == y);
    }
  public:
    using difference_type = long long;
    using value_type = typename ListElementType::ValueType;
    using pointer = value_type*;
    using reference = value_type&;
    using iterator_category = std::bidirectional_iterator_tag;
  private:
    typename ListType::iterator mListIterator;
    typename ListType::iterator mEnd;
  public:
    explicit Iterator(typename ListType::iterator it, typename ListType::iterator end) noexcept;
    Iterator(const Iterator&) = default;
    Iterator(Iterator&&) = default;
    Iterator& operator=(const Iterator&) = default;
    Iterator& operator=(Iterator&&) = default;
    ~Iterator() = default;
    reference operator*() noexcept;
    pointer operator->() noexcept;
    Iterator& operator++() noexcept;
    Iterator operator++(int) noexcept;
  };
  class ConstIterator
  {
    friend bool operator==(const ConstIterator& x, const ConstIterator& y)
    {
      return x.mListIterator == y.mListIterator;
    }
    friend bool operator!=(const ConstIterator& x, const ConstIterator& y)
    {
      return !(x == y);
    }
  public:
    using difference_type = typename Iterator::difference_type;
    using reference = const typename Iterator::value_type&;
    using value_type = typename Iterator::value_type;
    using pointer = const value_type*;
    using iterator_category = std::bidirectional_iterator_tag;
  private:
    typename ListType::const_iterator mListIterator;
    typename ListType::const_iterator mEnd;
  public:
    explicit ConstIterator(typename ListType::const_iterator it, typename ListType::const_iterator end) noexcept;
    ConstIterator(const ConstIterator&) = default;
    ConstIterator(ConstIterator&&) = default;
    ConstIterator& operator=(const ConstIterator&) = default;
    ConstIterator& operator=(ConstIterator&&) = default;
    ~ConstIterator() = default;
    reference operator*() noexcept;
    pointer operator->() noexcept;
    ConstIterator& operator++() noexcept;
    ConstIterator operator++(int) noexcept;
  };

public:
  using key_type = Key;
  using value_type = typename Iterator::value_type;
  using mapped_type = T;
  using hasher = Hash;
  using key_equal = Pred;
  using pointer = typename Iterator::pointer;
  using const_pointer = typename ConstIterator::pointer;
  using reference = typename Iterator::reference;
  using const_reference = typename ConstIterator::reference;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using iterator = Iterator;
  using const_iterator = ConstIterator;

private:
  ListType mSplitOrderedList;
  std::vector<typename ListType::iterator> mBaseArray;
  size_type mSize;
  size_type mLevel;
  hasher mHash;

private:
  static constexpr std::size_t sThreshold = 4;
  static constexpr HashValueType sHiMask = (static_cast<HashValueType>(1) << (sizeof(HashValueType) * 8 - 1));
  static constexpr HashValueType sMask = ~sHiMask;
  static constexpr HashValueType reverse(HashValueType h);
  static constexpr HashValueType makeSentinelKey(HashValueType h);
  HashValueType makeBaseArrayMask() const noexcept;
  HashValueType makeOrdinaryKey(const key_type& k);
  HashValueType prevBaseArrayIndex(HashValueType index) const noexcept;
  typename ListType::iterator begin(HashValueType index);
  typename ListType::iterator begin(const key_type& k);
  template <typename U>
  std::pair<iterator, bool> insertImpl(U&& v);

public:
  SplitOrderedHashMap();
  iterator begin() noexcept;
  iterator end() noexcept;
  const_iterator begin() const noexcept;
  const_iterator end() const noexcept;
  size_type size() const noexcept;
  bool empty() const noexcept;
  std::pair<iterator, bool> insert(const value_type& v);
  std::pair<iterator, bool> insert(value_type&& v);
  size_type erase(const key_type& k);
  iterator find(const key_type& k);
  const_iterator find(const key_type& k) const;

};

}
}

#include "SplitOrderedHashMap.inl"

#endif

