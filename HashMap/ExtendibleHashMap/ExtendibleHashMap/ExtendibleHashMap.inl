#include <cassert>
#include <cstring>

namespace My
{
namespace ExtendibleHashMap
{
namespace Detail
{
template <typename T, std::size_t Length>
DataPage<T, Length>::DataPage(std::size_t localDepth) noexcept :
  BaseType(),
  mLocalDepth(localDepth)
{
}
template <typename T, std::size_t Length>
void DataPage<T, Length>::swap(DataPage& other) noexcept(std::is_nothrow_swappable_v<value_type>)
{
  using std::swap;
  swap(*this, other);
  swap(this->mLocalDepth, other.mLocalDepth);
}
template <typename T, std::size_t Length>
std::size_t DataPage<T, Length>::getLocalDepth() const noexcept
{
  return mLocalDepth;
}
template <typename T, std::size_t Length>
void DataPage<T, Length>::setLocalDepth(std::size_t localDepth) noexcept
{
  mLocalDepth = localDepth;
}

}  // namespace Detail

#define EXTENDIBLE_HASHMAP_TEMPLATE_PARAM_DECL \
  template <typename Key, typename T, typename Hash, typename Pred>
#define EXTENDIBLE_HASHMAP ExtendibleHashMap<Key, T, Hash, Pred>

EXTENDIBLE_HASHMAP_TEMPLATE_PARAM_DECL
typename EXTENDIBLE_HASHMAP::DataPagePointer EXTENDIBLE_HASHMAP::makeDataPage(std::size_t localDepth)
{
  mCapacity += sDataPageSize;
  return std::make_shared<DataPage>(localDepth);
}
EXTENDIBLE_HASHMAP_TEMPLATE_PARAM_DECL
typename EXTENDIBLE_HASHMAP::HashValue EXTENDIBLE_HASHMAP::makeMask(std::size_t depth) noexcept
{
  HashValue ans = 0;
  for(std::size_t i = 0; i < depth; ++i)
  {
    ans <<= 1;
    ans |= 1;
  }
  return ans;
}
EXTENDIBLE_HASHMAP_TEMPLATE_PARAM_DECL
void EXTENDIBLE_HASHMAP::rehash(HashValue hash, std::size_t localDepth)
{
  HashValue mask = makeMask(localDepth);
  auto dirIdx1 = hash & mask;
  auto& page1 = *mDirectoryArray[dirIdx1];
  assert(page1.getLocalDepth() == localDepth);
  auto dirIdx2 = dirIdx1 | (1 << page1.getLocalDepth());
  assert(dirIdx1 < mDirectoryArray.size());
  assert(page1.getLocalDepth() <= mGlobalDepth);
  // TODO: strong exception safety
  if(page1.getLocalDepth() < mGlobalDepth)
  {
    assert(dirIdx2 < mDirectoryArray.size());
    assert(mDirectoryArray[dirIdx1] == mDirectoryArray[dirIdx2]);
    page1.setLocalDepth(page1.getLocalDepth() + 1);
    mDirectoryArray[dirIdx2] = makeDataPage(page1.getLocalDepth());
    auto d = 1 << page1.getLocalDepth();
    for(std::size_t i = dirIdx1 + d; i < mDirectoryArray.size(); i += d)
    {
      mDirectoryArray[i] = mDirectoryArray[dirIdx1];
    }
    for(std::size_t i = dirIdx2 + d; i < mDirectoryArray.size(); i += d)
    {
      mDirectoryArray[i] = mDirectoryArray[dirIdx2];
    }
  }
  else
  {
    mGlobalDepth++;
    mDirectoryArray.resize(mDirectoryArray.size() * 2);
    assert(mDirectoryArray.size() == 1u << mGlobalDepth);
    for(std::size_t i = 0; i < mDirectoryArray.size() / 2; ++i)
    {
      mDirectoryArray[i | (1 << (mGlobalDepth - 1))] = mDirectoryArray[i];
    }
    mDirectoryArray[dirIdx1]->setLocalDepth(mGlobalDepth);
    mDirectoryArray[dirIdx2] = makeDataPage(mGlobalDepth);
  }
  auto& page2 = *mDirectoryArray[dirIdx2];
  mask |= 1 << localDepth;
  for(int i = page1.size() - 1; 0 <= i; --i)
  {
    auto h = mHasher(page1[i].first);
    if((h & mask) != dirIdx1)
    {
      page2.push_back(std::move(page1[i]));
      page1.remove(i);
    }
  }
}
EXTENDIBLE_HASHMAP_TEMPLATE_PARAM_DECL
EXTENDIBLE_HASHMAP::ExtendibleHashMap() :
  mDirectoryArray({makeDataPage(1), makeDataPage(1)}),
  mGlobalDepth(1),
  mSize(0)
{
}
EXTENDIBLE_HASHMAP_TEMPLATE_PARAM_DECL
bool EXTENDIBLE_HASHMAP::tryShrink()
{
  if(mDirectoryArray.size() <= 2)
  {
    return false;
  }
  for(std::size_t i = 0; i < mDirectoryArray.size() / 2; ++i)
  {
    std::size_t j = i | (1 << (mGlobalDepth - 1));
    auto& page1 = mDirectoryArray[i];
    auto& page2 = mDirectoryArray[j];
    if(page1 != page2 && sDataPageSize < page1->size() + page2->size())
    {
      return false;
    }
  }

  for(std::size_t i = 0; i < mDirectoryArray.size() / 2; ++i)
  {
    std::size_t j = i | (1 << (mGlobalDepth - 1));
    auto& page1 = mDirectoryArray[i];
    auto& page2 = mDirectoryArray[j];
    if(page1 != page2)
    {
      for(std::size_t k = 0; k < page2->size(); ++k)
      {
        page1->push_back(std::move((*page2)[k]));
      }
      page1->setLocalDepth(mGlobalDepth - 1);
    }
  }
  mDirectoryArray.resize(mDirectoryArray.size() / 2);
  --mGlobalDepth;
  return true;
}
EXTENDIBLE_HASHMAP_TEMPLATE_PARAM_DECL
template <typename U>
std::pair<typename EXTENDIBLE_HASHMAP::iterator, bool> EXTENDIBLE_HASHMAP::insertImpl(U&& v)
{
  static_assert(std::is_same_v<std::decay_t<U>, value_type>);
  auto hash = mHasher(v.first);
  auto globalMask = makeMask(mGlobalDepth);
  auto direcotryIndex = hash & globalMask;
  assert(direcotryIndex < mDirectoryArray.size());
  auto page = mDirectoryArray[direcotryIndex];
  Pred pred{};
  const auto& key = v.first;
  auto it = std::find_if(page->begin(), page->end(), [pred, &key](const value_type& x) {
    return pred(x.first, key);
  });
  if(it != page->end())
  {
    return std::make_pair(iterator{*this, it, direcotryIndex}, false);
  }
  auto localMask = makeMask(page->getLocalDepth());
  while(page->full())
  {
    rehash(hash, page->getLocalDepth());
    localMask |= 1 << (page->getLocalDepth() - 1);
    page = mDirectoryArray[hash & localMask];
  }
  page->push_back(std::forward<U>(v));
  ++mSize;
  return std::make_pair(iterator{*this, &page->back(), hash & localMask}, true);
}
EXTENDIBLE_HASHMAP_TEMPLATE_PARAM_DECL
std::pair<typename EXTENDIBLE_HASHMAP::iterator, bool> EXTENDIBLE_HASHMAP::insert(const value_type& v)
{
  return insertImpl(v);
}
EXTENDIBLE_HASHMAP_TEMPLATE_PARAM_DECL
std::pair<typename EXTENDIBLE_HASHMAP::iterator, bool> EXTENDIBLE_HASHMAP::insert(value_type&& v)
{
  return insertImpl(std::move(v));
}
EXTENDIBLE_HASHMAP_TEMPLATE_PARAM_DECL
typename EXTENDIBLE_HASHMAP::size_type EXTENDIBLE_HASHMAP::erase(const key_type& k)
{
  auto h = mHasher(k);
  auto mask = makeMask(mGlobalDepth);
  auto& page = *mDirectoryArray[h & mask];
  Pred pred{};
  auto it = std::find_if(page.begin(), page.end(), [pred, k](const value_type& x) {
    return pred(x.first, k);
  });
  if(it == page.end())
  {
    return 0;
  }
  page.remove(it - page.begin());
  --mSize;
  if(mSize < ((mDirectoryArray.size() / 2) * sDataPageSize) / 2)
  {
    tryShrink();
  }
  return 1;
}
EXTENDIBLE_HASHMAP_TEMPLATE_PARAM_DECL
typename EXTENDIBLE_HASHMAP::iterator EXTENDIBLE_HASHMAP::find(const key_type& k)
{
  auto hash = mHasher(k);
  auto dirIndex = hash & makeMask(mGlobalDepth);
  auto& page = *mDirectoryArray[dirIndex];
  Pred pred{};
  auto it = std::find_if(page.begin(), page.end(), [pred, &k](const value_type& v) {
    return pred(v.first, k);
  });
  return it != page.end() ? iterator(*this, it, dirIndex) : iterator{};
}
EXTENDIBLE_HASHMAP_TEMPLATE_PARAM_DECL
typename EXTENDIBLE_HASHMAP::iterator EXTENDIBLE_HASHMAP::begin() noexcept
{
  // TODO: cache begin
  if(empty())
  {
    return iterator{};
  }
  for(std::size_t i = 0; i < mDirectoryArray.size(); ++i)
  {
    auto& page = *mDirectoryArray[i];
    if(page.size())
    {
      return iterator{*this, page.begin(), i};
    }
  }
  return iterator{};
}
EXTENDIBLE_HASHMAP_TEMPLATE_PARAM_DECL
typename EXTENDIBLE_HASHMAP::iterator EXTENDIBLE_HASHMAP::end() noexcept
{
  return iterator{};
}
EXTENDIBLE_HASHMAP_TEMPLATE_PARAM_DECL
typename EXTENDIBLE_HASHMAP::size_type EXTENDIBLE_HASHMAP::size() const noexcept
{
  return mSize;
}
EXTENDIBLE_HASHMAP_TEMPLATE_PARAM_DECL
typename EXTENDIBLE_HASHMAP::size_type EXTENDIBLE_HASHMAP::capacity() const noexcept
{
  return mCapacity;
}
EXTENDIBLE_HASHMAP_TEMPLATE_PARAM_DECL
bool EXTENDIBLE_HASHMAP::empty() const noexcept
{
  return size() == 0;
}

EXTENDIBLE_HASHMAP_TEMPLATE_PARAM_DECL
EXTENDIBLE_HASHMAP::Iterator::Iterator() noexcept :
  mThis(nullptr),
  mCurrent(0),
  mDirectoryIndex(0)
{
}
EXTENDIBLE_HASHMAP_TEMPLATE_PARAM_DECL
EXTENDIBLE_HASHMAP::Iterator::Iterator(ThisType& self,
                                       pointer current,
                                       std::size_t direcotryIndex) noexcept :
  mThis(&self),
  mCurrent(current),
  mDirectoryIndex(direcotryIndex)
{
}
EXTENDIBLE_HASHMAP_TEMPLATE_PARAM_DECL
typename EXTENDIBLE_HASHMAP::Iterator::reference EXTENDIBLE_HASHMAP::Iterator::operator*() noexcept
{
  return *mCurrent;
}
EXTENDIBLE_HASHMAP_TEMPLATE_PARAM_DECL
typename EXTENDIBLE_HASHMAP::Iterator::pointer EXTENDIBLE_HASHMAP::Iterator::
operator->() noexcept
{
  return mCurrent;
}
EXTENDIBLE_HASHMAP_TEMPLATE_PARAM_DECL
typename EXTENDIBLE_HASHMAP::Iterator& EXTENDIBLE_HASHMAP::Iterator::operator++() noexcept
{
  auto& currentPage = *mThis->mDirectoryArray[mDirectoryIndex];
  if(mCurrent + 1 != currentPage.begin() + currentPage.size())
  {
    mCurrent++;
    return *this;
  }
  mDirectoryIndex++;
  for(; mDirectoryIndex < mThis->mDirectoryArray.size(); ++mDirectoryIndex)
  {
    auto& nextPage = *mThis->mDirectoryArray[mDirectoryIndex];
    mCurrent = nextPage.begin();
    if((nextPage.getLocalDepth() == mThis->mGlobalDepth) && !nextPage.empty())
    {
      return *this;
    }
    auto mask = makeMask(nextPage.getLocalDepth());
    if(((mDirectoryIndex & mask) == mDirectoryIndex) && !nextPage.empty())
    {
      return *this;
    }
  }
  (*this) = Iterator{};
  return *this;
}
EXTENDIBLE_HASHMAP_TEMPLATE_PARAM_DECL
typename EXTENDIBLE_HASHMAP::Iterator EXTENDIBLE_HASHMAP::Iterator::operator++(int) noexcept
{
  auto ret = *this;
  ++(*this);
  return ++(*this);
}

EXTENDIBLE_HASHMAP_TEMPLATE_PARAM_DECL
EXTENDIBLE_HASHMAP::ConstIterator::ConstIterator(const Iterator& iterator) noexcept :
  mIterator(iterator)
{
}
EXTENDIBLE_HASHMAP_TEMPLATE_PARAM_DECL
typename EXTENDIBLE_HASHMAP::ConstIterator::reference EXTENDIBLE_HASHMAP::ConstIterator::operator*() noexcept
{
  return *mIterator;
}
EXTENDIBLE_HASHMAP_TEMPLATE_PARAM_DECL
typename EXTENDIBLE_HASHMAP::ConstIterator::pointer
    EXTENDIBLE_HASHMAP::ConstIterator::operator->() noexcept
{
  return mIterator.operator->();
}
EXTENDIBLE_HASHMAP_TEMPLATE_PARAM_DECL
typename EXTENDIBLE_HASHMAP::ConstIterator& EXTENDIBLE_HASHMAP::ConstIterator::operator++() noexcept
{
  mIterator++;
  return *this;
}
EXTENDIBLE_HASHMAP_TEMPLATE_PARAM_DECL
typename EXTENDIBLE_HASHMAP::ConstIterator EXTENDIBLE_HASHMAP::ConstIterator::operator++(int) noexcept
{
  auto ret = *this;
  ++(*this);
  return ret;
}

#undef EXTENDIBLE_HASHMAP_TEMPLATE_PARAM_DECL
#undef EXTENDIBLE_HASHMAP

}
}  // namespace My
