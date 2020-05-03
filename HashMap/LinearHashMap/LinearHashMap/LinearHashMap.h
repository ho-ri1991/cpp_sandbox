#ifndef LINEAR_HASHMAP_H
#define LINEAR_HASHMAP_H

#include "Common/DataPage.h"
#include <vector>

namespace My
{
namespace LinearHashMap
{
namespace Detail
{
template <typename T, std::size_t Length>
class DataPage
{
private:
  using BaseType = Common::DataPage<T, Length>;
  using ThisType = DataPage;
  struct Iterator
  {
    friend struct ConstIterator;
    friend bool operator==(const Iterator& x, const Iterator& y)
    {
      return x.mThis == y.mThis && x.mCurrent == y.mCurrent;
    }
    friend bool operator!=(const Iterator& x, const Iterator& y)
    {
      return !(x == y);
    }
    static constexpr std::size_t sLength = Length;
  public:
    using difference_type = long long;
    using value_type = typename BaseType::value_type;
    using pointer = typename BaseType::pointer;
    using reference = typename BaseType::reference;
    using iterator_category = std::bidirectional_iterator_tag;
  private:
    ThisType* mThis;
    std::size_t mCurrent;
  public:
    Iterator() noexcept;
    explicit Iterator(ThisType* self, std::size_t current) noexcept;
    Iterator(const Iterator&) = default;
    Iterator(Iterator&&) = default;
    Iterator& operator=(const Iterator&) = default;
    Iterator& operator=(Iterator&&) = default;
    ~Iterator() = default;
    reference operator*() noexcept;
    pointer operator->() noexcept;
    Iterator& operator++() noexcept;
    Iterator operator++(int) noexcept;
    std::size_t getCurrent() const noexcept;
  };
  struct ConstIterator
  {
    friend bool operator==(const ConstIterator& x, const ConstIterator& y)
    {
      return x.mThis == y.mThis && x.mCurrent == y.mCurrent;
    }
    friend bool operator!=(const ConstIterator& x, const ConstIterator& y)
    {
      return !(x == y);
    }
    static constexpr std::size_t sLength = Length;
  public:
    using difference_type = long long;
    using value_type = typename BaseType::value_type;
    using pointer = typename BaseType::const_pointer;
    using reference = typename BaseType::const_reference;
    using iterator_category = std::bidirectional_iterator_tag;
  private:
    const ThisType* mThis;
    std::size_t mCurrent;
  public:
    ConstIterator() noexcept;
    ConstIterator(Iterator it) noexcept;
    explicit ConstIterator(const ThisType* self, std::size_t current) noexcept;
    ConstIterator(const ConstIterator&) = default;
    ConstIterator(ConstIterator&&) = default;
    ConstIterator& operator=(const ConstIterator&) = default;
    ConstIterator& operator=(ConstIterator&&) = default;
    ~ConstIterator() = default;
    reference operator*() noexcept;
    pointer operator->() noexcept;
    ConstIterator& operator++() noexcept;
    ConstIterator operator++(int) noexcept;
    std::size_t getCurrent() const noexcept;
  };

public:
  static constexpr std::size_t sLength = Length;
  using value_type = typename Iterator::value_type;
  using pointer = typename Iterator::pointer;
  using const_pointer = typename ConstIterator::pointer;
  using iterator = Iterator;
  using const_iterator = ConstIterator;
  using reference = typename Iterator::reference;
  using const_reference = typename ConstIterator::reference;
  using size_type = std::size_t;

private:
  BaseType mBasePage;
  std::vector<value_type> mOverflowPage;

public:
  DataPage() = default;
  void swap(DataPage& other) noexcept(std::is_nothrow_swappable_v<value_type>);
  size_type size() const noexcept;
  bool empty() const noexcept;
  reference operator[](size_type n) noexcept;
  const_reference operator[](size_type n) const noexcept;
  reference get(size_type n) noexcept;
  const_reference get(size_type n) const noexcept;
  iterator begin() noexcept;
  iterator end() noexcept;
  const_iterator begin() const noexcept;
  const_iterator end() const noexcept;
  void push_back(const value_type& value);
  void push_back(value_type&& value);
  template <typename... Args>
  void emplace_back(Args&&... args);
  void remove(size_type n);
  void remove(iterator it);
  void remove(const_iterator it);
  void pop_back() noexcept;
  reference back() noexcept;
  const_reference back() const noexcept;
};
}

template <typename Key, typename T, typename Hash = std::hash<Key>, typename Pred = std::equal_to<Key> /*TODO: Allocator*/>
class LinearHashMap
{
private:
  using ThisType = LinearHashMap;
  class Iterator
  {
    friend bool operator==(const Iterator& x, const Iterator& y)
    {
      return x.mThis == y.mThis && x.mCurrent == y.mCurrent &&
             x.mPageIndex == y.mPageIndex;
    }
    friend bool operator!=(const Iterator& x, const Iterator& y)
    {
      return !(x == y);
    }
  public:
    using difference_type = long long;
    using value_type = std::pair<Key, T>;
    using pointer = value_type*;
    using reference = value_type&;
    using iterator_category = std::bidirectional_iterator_tag;
  private:
    ThisType* mThis;
    std::size_t mPageIndex;
    std::size_t mCurrent;
  public:
    Iterator() noexcept;
    Iterator(ThisType* self, std::size_t pageIndex, std::size_t current) noexcept;
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
      return x.mThis == y.mThis && x.mCurrent == y.mCurrent &&
             x.mPageIndex == y.mPageIndex;
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
    ThisType* mThis;
    std::size_t mPageIndex;
    std::size_t mCurrent;
  public:
    ConstIterator() noexcept;
    ConstIterator(ThisType* self, std::size_t pageIndex, std::size_t current) noexcept;
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
  using value_type = typename Iterator::value_type;  // TODO: make Key const
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
  static constexpr std::size_t sInitialLevel = 5;
  static constexpr std::size_t sDataPageSize = 4;
  using HashValue =
      decltype(std::declval<const hasher&>()(std::declval<const key_type&>()));
  static_assert(std::is_integral_v<HashValue>);
  using DataPage = Detail::DataPage<value_type, sDataPageSize>;
  using Container = std::vector<DataPage>;
  Container mDataPages;
  std::size_t mNext;
  std::size_t mLevel;
  std::size_t mSize;
  hasher mHasher; // TODO: use EBO

private:
  std::size_t makeMask();
  std::size_t makePageIndex(const key_type& k);
  template <typename U>
  std::pair<iterator, bool> insertImpl(U&& v);

public:
  LinearHashMap();
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

#include "LinearHashMap.inl"

#endif

