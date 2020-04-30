#ifndef HASHMAP_COMMON_DATAPAGE_H
#define HASHMAP_COMMON_DATAPAGE_H

#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>

namespace My
{
namespace Common
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

private:
  pointer getPointer() noexcept;
  const_pointer getPointer() const noexcept;
  void destruct(size_t n) noexcept;
  static void destroy(pointer p) noexcept;

public:
  DataPage() noexcept;
  DataPage(const DataPage& other) noexcept(std::is_nothrow_copy_constructible_v<value_type>);
  DataPage(DataPage&& other) noexcept(std::is_nothrow_move_constructible_v<value_type>);
  DataPage& operator=(const DataPage& other);
  DataPage& operator=(DataPage&& other) noexcept(std::is_nothrow_move_constructible_v<value_type>);
  ~DataPage() noexcept;
  void swap(DataPage& other) noexcept(std::is_nothrow_swappable_v<value_type>);
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
}  // namespace Common
}  // namespace My

#include "DataPage.inl"

#endif
