#include <cstring>
#include <stdexcept>

namespace My
{
namespace Common
{
template <typename T, std::size_t Length>
typename DataPage<T, Length>::pointer DataPage<T, Length>::getPointer() noexcept
{
  // TODO: needs std::launder?
  return reinterpret_cast<pointer>(mData);
}
template <typename T, std::size_t Length>
typename DataPage<T, Length>::const_pointer DataPage<T, Length>::getPointer() const noexcept
{
  // TODO: needs std::launder?
  return reinterpret_cast<const_pointer>(mData);
}
template <typename T, std::size_t Length>
void DataPage<T, Length>::destruct(std::size_t n) noexcept
{
  destroy(getPointer() + n);
}
template <typename T, std::size_t Length>
void DataPage<T, Length>::destroy(pointer p) noexcept
{
  if constexpr(!std::is_trivially_destructible_v<value_type>)
  {
    p->~value_type();
  }
}

template <typename T, std::size_t Length>
DataPage<T, Length>::DataPage() noexcept : mSize(0)
{
}
template <typename T, std::size_t Length>
DataPage<T, Length>::DataPage(const DataPage& other) noexcept(
    std::is_nothrow_copy_constructible_v<value_type>) :
  mSize(0)
{
  if constexpr(std::is_trivially_copyable_v<value_type>)
  {
    std::memcpy(this->mData, other.mData, sDataSizeInBytes);
    mSize = other.mSize;
  }
  else
  {
    for(std::size_t i = 0; i < other.size(); ++i)
    {
      this->push_back(other[i]);
    }
  }
}
template <typename T, std::size_t Length>
DataPage<T, Length>::DataPage(DataPage&& other) noexcept(std::is_nothrow_move_constructible_v<value_type>) :
  mSize(0)
{
  if constexpr(std::is_trivially_copyable_v<value_type>)
  {
    std::memcpy(this->mData, other.mData, sDataSizeInBytes);
    mSize = other.mSize;
  }
  else
  {
    for(std::size_t i = 0; i < other.size(); ++i)
    {
      this->push_back(std::move(other[i]));
    }
  }
}
template <typename T, std::size_t Length>
DataPage<T, Length>& DataPage<T, Length>::operator=(const DataPage& other)
{
  DataPage tmp(other);
  this->swap(tmp);
  return *this;
}
template <typename T, std::size_t Length>
DataPage<T, Length>& DataPage<T, Length>::operator=(DataPage&& other) noexcept(
    std::is_nothrow_move_constructible_v<value_type>)
{
  DataPage tmp(std::move(other));
  this->swap(tmp);
  return *this;
}
template <typename T, std::size_t Length>
DataPage<T, Length>::~DataPage() noexcept
{
  for(std::size_t i = 0; i < mSize; ++i)
  {
    destruct(i);
  }
}
template <typename T, std::size_t Length>
void DataPage<T, Length>::swap(DataPage& other) noexcept(std::is_nothrow_swappable_v<value_type>)
{
  if constexpr(std::is_swappable_v<value_type>)
  {
    using std::swap;
    if(this->mSize == other.mSize)
    {
      for(std::size_t i = 0; i < mSize; ++i)
      {
        swap((*this)[i], other[i]);
      }
    }
    else
    {
      DataPage& small = this->mSize < other.mSize ? *this : other;
      DataPage& large = this->mSize < other.mSize ? other : *this;
      for(std::size_t i = 0; i < small.size(); ++i)
      {
        swap(small[i], large[i]);
      }
      auto limit = large.size();
      for(std::size_t i = small.size(); i < limit; ++i)
      {
        small.push_back(std::move(large[i]));
        large.destruct(i);
        --large.mSize;
      }
    }
  }
  else
  {
    alignas(value_type) std::byte tmpStorage[sDataSizeInBytes];
    DataPage& small = this->size() < other.size() ? *this : other;
    DataPage& large = this->size() < other.size() ? other : *this;
    for(std::size_t i = 0; i < small.size(); ++i)
    {
      new(reinterpret_cast<value_type*>(tmpStorage) + i)
          value_type(std::move(small[i]));
    }
    for(std::size_t i = 0; i < small.size(); ++i)
    {
      small[i] = std::move(other[i]);
    }
    for(std::size_t i = small.size(); i < other.size(); ++i)
    {
      new(small.getPointer() + i) value_type(std::move(other[i]));
      other.destruct(i);
    }
    for(std::size_t i = 0; i < small.size(); ++i)
    {
      auto p = reinterpret_cast<value_type*>(tmpStorage) + i;
      large[i] = std::move(*p);
      destroy(p);
    }
    using std::swap;
    swap(small.mSize, large.mSize);
  }
}

template <typename T, std::size_t Length>
typename DataPage<T, Length>::size_type DataPage<T, Length>::size() const noexcept
{
  return mSize;
}
template <typename T, std::size_t Length>
bool DataPage<T, Length>::empty() const noexcept
{
  return mSize == 0;
}
template <typename T, std::size_t Length>
typename DataPage<T, Length>::reference DataPage<T, Length>::get(size_type n) noexcept
{
  return getPointer()[n];
}
template <typename T, std::size_t Length>
typename DataPage<T, Length>::const_reference DataPage<T, Length>::get(size_type n) const noexcept
{
  return getPointer()[n];
}
template <typename T, std::size_t Length>
typename DataPage<T, Length>::reference DataPage<T, Length>::operator[](size_type n) noexcept
{
  return getPointer()[n];
}
template <typename T, std::size_t Length>
typename DataPage<T, Length>::const_reference DataPage<T, Length>::operator[](size_type n) const
    noexcept
{
  return getPointer()[n];
}
template <typename T, std::size_t Length>
typename DataPage<T, Length>::pointer DataPage<T, Length>::begin() noexcept
{
  return getPointer();
}
template <typename T, std::size_t Length>
typename DataPage<T, Length>::pointer DataPage<T, Length>::end() noexcept
{
  return getPointer() + mSize;
}
template <typename T, std::size_t Length>
typename DataPage<T, Length>::const_pointer DataPage<T, Length>::begin() const noexcept
{
  return getPointer();
}
template <typename T, std::size_t Length>
typename DataPage<T, Length>::const_pointer DataPage<T, Length>::end() const noexcept
{
  return getPointer() + mSize;
}
template <typename T, std::size_t Length>
void DataPage<T, Length>::push_back(const value_type& value)
{
  if(sLength <= mSize)
  {
    throw std::out_of_range("");
  }
  new(getPointer() + mSize) value_type(value);
  ++mSize;
}
template <typename T, std::size_t Length>
void DataPage<T, Length>::push_back(value_type&& value)
{
  if(sLength <= mSize)
  {
    throw std::out_of_range("");
  }
  new(getPointer() + mSize) value_type(std::move(value));
  ++mSize;
}
template <typename T, std::size_t Length>
template <typename... Args>
void DataPage<T, Length>::emplace_back(Args&&... args)
{
  if(sLength <= mSize)
  {
    throw std::out_of_range("");
  }
  new(getPointer() + mSize) value_type(std::forward<Args>(args)...);
  ++mSize;
}
template <typename T, std::size_t Length>
void DataPage<T, Length>::remove(size_type n)
{
  if(mSize <= n)
  {
    throw std::out_of_range("");
  }
  if(mSize == n + 1)
  {
    destruct(n);
  }
  else
  {
    if constexpr(std::is_swappable_v<value_type>)
    {
      using std::swap;
      swap(getPointer()[n], getPointer()[mSize - 1]);
    }
    else
    {
      getPointer()[n] = std::move(getPointer()[mSize - 1]);
    }
    destruct(mSize - 1);
  }
  --mSize;
}
template <typename T, std::size_t Length>
void DataPage<T, Length>::pop_back()
{
  destruct(mSize - 1);
  mSize--;
}
template <typename T, std::size_t Length>
bool DataPage<T, Length>::full() const noexcept
{
  return mSize == sLength;
}
template <typename T, std::size_t Length>
typename DataPage<T, Length>::reference DataPage<T, Length>::back() noexcept
{
  return (*this)[size() - 1];
}
template <typename T, std::size_t Length>
typename DataPage<T, Length>::const_reference DataPage<T, Length>::back() const noexcept
{
  return (*this)[size() - 1];
}

template <typename T, std::size_t Length>
void swap(DataPage<T, Length>& x, DataPage<T, Length>& y) noexcept(std::is_nothrow_swappable_v<typename DataPage<T, Length>::value_type>)
{
  x.swap(y);
}
}  // namespace Common
}  // namespace My
