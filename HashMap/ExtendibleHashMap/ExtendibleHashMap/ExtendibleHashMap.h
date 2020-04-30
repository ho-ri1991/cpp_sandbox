#include <cstddef>
#include <functional>
#include <iterator>
#include <limits>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace My
{
namespace Detail
{
template <typename T, std::size_t Length>
class DataPage
{
public:
  static constexpr std::size_t sLength = Length;
  using value_type = T;
  using pointer = value_type*;
  using const_pointer = const value_type*;
  using reference = value_type&;
  using const_reference = const value_type&;
  using size_type = std::conditional_t<
      sLength <= std::numeric_limits<unsigned char>::max(),
      unsigned char,
      std::conditional_t<sLength <= std::numeric_limits<unsigned short>::max(),
                         unsigned short,
                         std::conditional_t<sLength <= std::numeric_limits<unsigned int>::max(),
                                            unsigned int,
                                            std::conditional_t<sLength <= std::numeric_limits<unsigned long>::max(), unsigned long, unsigned long long>>>>;

private:
  static constexpr std::size_t sDataSizeInBytes = sizeof(value_type) * sLength;
  alignas(value_type) std::byte mData[sDataSizeInBytes];
  size_type mSize;
  std::size_t mLocalDepth;

private:
  pointer getPointer() noexcept;
  const_pointer getPointer() const noexcept;
  void destruct(size_t n) noexcept;
  static void destroy(pointer p) noexcept;

public:
  DataPage(std::size_t localDepth) noexcept;
  DataPage(const DataPage& other) noexcept(std::is_nothrow_copy_constructible_v<value_type>);
  DataPage(DataPage&& other) noexcept(std::is_nothrow_move_constructible_v<value_type>);
  DataPage& operator=(const DataPage& other);
  DataPage& operator=(DataPage&& other) noexcept(std::is_nothrow_move_constructible_v<value_type>);
  ~DataPage() noexcept;
  void swap(DataPage& other) noexcept(std::is_nothrow_swappable_v<value_type>);
  std::size_t getLocalDepth() const noexcept;
  void setLocalDepth(std::size_t localDepth) noexcept;
  size_type size() const noexcept;
  bool empty() const noexcept;
  reference operator[](size_type n) noexcept;
  const_reference operator[](size_type n) const noexcept;
  reference get(size_type n) noexcept;
  const_reference get(size_type n) const noexcept;
  pointer begin() noexcept;
  pointer end() noexcept;
  const_pointer begin() const noexcept;
  const_pointer end() const noexcept;
  void push_back(const value_type& value);
  void push_back(value_type&& value);
  template <typename... Args>
  void emplace_back(Args&&... args);
  void remove(size_type n);
  void pop_back();
  bool full() const noexcept;
  reference back() noexcept;
  const_reference back() const noexcept;
};
}  // namespace Detail

template <typename Key, typename T, typename Hash = std::hash<Key>, typename Pred = std::equal_to<Key> /*TODO: Allocator*/>
class ExtendibleHashMap
{
private:
  using ThisType = ExtendibleHashMap;
  class Iterator
  {
    friend bool operator==(const Iterator& x, const Iterator& y)
    {
      return x.mThis == y.mThis && x.mCurrent == y.mCurrent &&
             x.mDirectoryIndex == y.mDirectoryIndex;
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
    pointer mCurrent;
    std::size_t mDirectoryIndex;

  public:
    Iterator() noexcept;
    Iterator(ThisType& self, pointer current, std::size_t directoryIndex) noexcept;
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
  public:
    using difference_type = typename Iterator::difference_type;
    using reference = const typename Iterator::value_type&;
    using value_type = typename Iterator::value_type;
    using pointer = const value_type*;
    using iterator_category = std::bidirectional_iterator_tag;

  private:
    Iterator mIterator;

  public:
    ConstIterator(const Iterator& iterator) noexcept;
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

private:
  static constexpr std::size_t sDataPageSize = 4;
  using HashValue =
      decltype(std::declval<const hasher&>()(std::declval<const key_type&>()));
  static_assert(std::is_integral_v<HashValue>);
  using DataPage = Detail::DataPage<value_type, sDataPageSize>;
  using DataPagePointer = std::shared_ptr<DataPage>;  // TODO: use more light weight pointer
  using DirectoryArray = std::vector<DataPagePointer>;
  DirectoryArray mDirectoryArray;
  std::size_t mGlobalDepth;
  std::size_t mSize;
  std::size_t mCapacity;
  hasher mHasher;  // TODO: use EBO
public:
  using iterator = Iterator;
  using const_iterator = ConstIterator;
  /*TODO: reverse_iterator*/
private:
  DataPagePointer makeDataPage(std::size_t localDepth);
  static HashValue makeMask(std::size_t depth) noexcept;
  void rehash(HashValue hash, std::size_t localDepth);
  template <typename U>
  std::pair<iterator, bool> insertImpl(U&& v);
  bool tryShrink();

public:
  ExtendibleHashMap();
  iterator begin() noexcept;
  iterator end() noexcept;
  const_iterator begin() const noexcept;
  const_iterator end() const noexcept;
  size_type size() const noexcept;
  size_type capacity() const noexcept;
  bool empty() const noexcept;
  std::pair<iterator, bool> insert(const value_type& v);
  std::pair<iterator, bool> insert(value_type&& v);
  size_type erase(const key_type& k);
  iterator find(const key_type& k);
  //  const_iterator find(const key_type& k) const;
};

}  // namespace My

#include "ExtendibleHashMap.inl"
