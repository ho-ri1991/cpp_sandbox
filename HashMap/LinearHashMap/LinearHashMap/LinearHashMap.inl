#include <type_traits>

namespace My
{
namespace LinearHashMap
{
namespace Detail
{
#define DATAPAGE_TEMPLATE_PARAM_DECL \
  template <typename T, std::size_t Length>
#define DATAPAGE DataPage<T, Length>

DATAPAGE_TEMPLATE_PARAM_DECL
DATAPAGE::Iterator::Iterator() noexcept:
  mThis(nullptr),
  mCurrent(0)
{
}
DATAPAGE_TEMPLATE_PARAM_DECL
DATAPAGE::Iterator::Iterator(ThisType* self, std::size_t current) noexcept:
  mThis(self),
  mCurrent(current)
{
}
DATAPAGE_TEMPLATE_PARAM_DECL
typename DATAPAGE::Iterator::reference DATAPAGE::Iterator::operator*() noexcept
{
  return mCurrent < sLength ? mThis->mBasePage[mCurrent] : mThis->mOverflowPage[mCurrent - sLength];
}
DATAPAGE_TEMPLATE_PARAM_DECL
typename DATAPAGE::Iterator::pointer DATAPAGE::Iterator::operator->() noexcept
{
  return mCurrent < sLength ? &mThis->mBasePage[mCurrent] : &mThis->mOverflowPage[mCurrent - sLength];
}
DATAPAGE_TEMPLATE_PARAM_DECL
typename DATAPAGE::Iterator& DATAPAGE::Iterator::operator++() noexcept
{
  ++mCurrent;
  if(mThis->size() <= mCurrent)
  {
    mCurrent = 0;
    mThis = nullptr;
  }
  return *this;
}
DATAPAGE_TEMPLATE_PARAM_DECL
typename DATAPAGE::Iterator DATAPAGE::Iterator::operator++(int) noexcept
{
  auto res = *this;
  ++(*this);
  return res;
}
DATAPAGE_TEMPLATE_PARAM_DECL
std::size_t DATAPAGE::Iterator::getCurrent() const noexcept
{
  return mCurrent;
}

DATAPAGE_TEMPLATE_PARAM_DECL
DATAPAGE::ConstIterator::ConstIterator() noexcept:
  mThis(nullptr),
  mCurrent(0)
{
}
DATAPAGE_TEMPLATE_PARAM_DECL
DATAPAGE::ConstIterator::ConstIterator(Iterator it) noexcept:
  mThis(it.mThis),
  mCurrent(it.mCurrent)
{
}
DATAPAGE_TEMPLATE_PARAM_DECL
DATAPAGE::ConstIterator::ConstIterator(const ThisType* self, std::size_t current) noexcept:
  mThis(self),
  mCurrent(current)
{
}
DATAPAGE_TEMPLATE_PARAM_DECL
typename DATAPAGE::ConstIterator::reference DATAPAGE::ConstIterator::operator*() noexcept
{
  return mCurrent < sLength ? mThis->mBasePage[mCurrent] : mThis->mOverflowPage[mCurrent - sLength];
}
DATAPAGE_TEMPLATE_PARAM_DECL
typename DATAPAGE::ConstIterator::pointer DATAPAGE::ConstIterator::operator->() noexcept
{
  return mCurrent < sLength ? &mThis->mBasePage[mCurrent] : &mThis->mOverflowPage[mCurrent - sLength];
}
DATAPAGE_TEMPLATE_PARAM_DECL
typename DATAPAGE::ConstIterator& DATAPAGE::ConstIterator::operator++() noexcept
{
  ++mCurrent;
  if(mThis->size() <= mCurrent)
  {
    mCurrent = 0;
    mThis = nullptr;
  }
  return *this;
}
DATAPAGE_TEMPLATE_PARAM_DECL
typename DATAPAGE::ConstIterator DATAPAGE::ConstIterator::operator++(int) noexcept
{
  auto res = *this;
  ++(*this);
  return res;
}
DATAPAGE_TEMPLATE_PARAM_DECL
std::size_t DATAPAGE::ConstIterator::getCurrent() const noexcept
{
  return mCurrent;
}

DATAPAGE_TEMPLATE_PARAM_DECL
void DATAPAGE::swap(DataPage& other) noexcept(std::is_nothrow_swappable_v<value_type>)
{
  using std::swap;
  swap(this->mBasePage, other.mBasePage);
  swap(this->mOverflowPage, other.mOverflowPage);
  for(std::size_t i = this->mOverflowPage.size(); 0 < i; --i)
  {
    if(this->mBasePage.full())
    {
      break;
    }
    this->mBasePage.push_back(std::move(this->mOverflowPage[i - 1]));
    this->mOverflowPage.pop_back();
  }
  for(std::size_t i = other.mOverflowPage.size(); 0 < i; --i)
  {
    if(other.mBasePage.full())
    {
      break;
    }
    other.mBasePage.push_back(std::move(other.mOverflowPage[i - 1]));
    other.mOverflowPage.pop_back();
  }
}
DATAPAGE_TEMPLATE_PARAM_DECL
typename DATAPAGE::size_type DATAPAGE::size() const noexcept
{
  return mBasePage.size() + mOverflowPage.size();
}
DATAPAGE_TEMPLATE_PARAM_DECL
bool DATAPAGE::empty() const noexcept
{
  return mBasePage.empty();
}
DATAPAGE_TEMPLATE_PARAM_DECL
typename DATAPAGE::reference DATAPAGE::operator[](size_type n) noexcept
{
  return n < sLength ? mBasePage[n] : mOverflowPage[n - sLength];
}
DATAPAGE_TEMPLATE_PARAM_DECL
typename DATAPAGE::const_reference DATAPAGE::operator[](size_type n) const noexcept
{
  return n < sLength ? mBasePage[n] : mOverflowPage[n - sLength];
}
DATAPAGE_TEMPLATE_PARAM_DECL
typename DATAPAGE::reference DATAPAGE::get(size_type n) noexcept
{
  return (*this)[n];
}
DATAPAGE_TEMPLATE_PARAM_DECL
typename DATAPAGE::const_reference DATAPAGE::get(size_type n) const noexcept
{
  return (*this)[n];
}
DATAPAGE_TEMPLATE_PARAM_DECL
typename DATAPAGE::iterator DATAPAGE::begin() noexcept
{
  return mBasePage.empty() ? end() : iterator(this, 0);
}
DATAPAGE_TEMPLATE_PARAM_DECL
typename DATAPAGE::iterator DATAPAGE::end() noexcept
{
  return iterator{};
}
DATAPAGE_TEMPLATE_PARAM_DECL
typename DATAPAGE::const_iterator DATAPAGE::begin() const noexcept
{
  return mBasePage.empty() ? end() : const_iterator(this, 0);
}
DATAPAGE_TEMPLATE_PARAM_DECL
typename DATAPAGE::const_iterator DATAPAGE::end() const noexcept
{
  return const_iterator{};
}
DATAPAGE_TEMPLATE_PARAM_DECL
void DATAPAGE::push_back(const value_type& value)
{
  if(!mBasePage.full())
  {
    mBasePage.push_back(value);
  }
  else
  {
    mOverflowPage.push_back(value);
  }
}
DATAPAGE_TEMPLATE_PARAM_DECL
void DATAPAGE::push_back(value_type&& value)
{
  if(!mBasePage.full())
  {
    mBasePage.push_back(std::move(value));
  }
  else
  {
    mOverflowPage.push_back(std::move(value));
  }
}
DATAPAGE_TEMPLATE_PARAM_DECL
template <typename... Args>
void DATAPAGE::emplace_back(Args&&... args)
{
  if(!mBasePage.full())
  {
    mBasePage.emplace_back(std::forward<Args>(args)...);
  }
  else
  {
    mOverflowPage.emplace_back(std::forward<Args>(args)...);
  }
}
DATAPAGE_TEMPLATE_PARAM_DECL
void DATAPAGE::remove(size_type n)
{
  if(size() <= n)
  {
    throw std::out_of_range("");
  }
  pointer p = nullptr;
  if(n < sLength)
  {
    if(mOverflowPage.empty())
    {
      mBasePage.remove(n);
      return;
    }
    else
    {
      p = &mBasePage[n];
    }
  }
  else
  {
    p = &mOverflowPage[n - sLength];
  }
  assert(p);
  assert(mBasePage.full());
  assert(!mOverflowPage.empty());
  if(p == &mOverflowPage.back())
  {
    mOverflowPage.pop_back();
    return;
  }
  if constexpr(std::is_swappable_v<value_type>)
  {
    using std::swap;
    swap(*p, mOverflowPage.back());
  }
  else
  {
    *p = std::move(mOverflowPage.back());
  }
  mOverflowPage.pop_back();
}
DATAPAGE_TEMPLATE_PARAM_DECL
void DATAPAGE::remove(iterator it)
{
  remove(it.getCurrent());
}
DATAPAGE_TEMPLATE_PARAM_DECL
void DATAPAGE::pop_back() noexcept
{
  remove(size() - 1);
}
DATAPAGE_TEMPLATE_PARAM_DECL
typename DATAPAGE::reference DATAPAGE::back() noexcept
{
  return mOverflowPage.empty() ? mBasePage.back() : mOverflowPage.back();
}
DATAPAGE_TEMPLATE_PARAM_DECL
typename DATAPAGE::const_reference DATAPAGE::back() const noexcept
{
  return mOverflowPage.empty() ? mBasePage.back() : mOverflowPage.back();
}

#undef DATAPAGE_TEMPLATE_PARAM_DECL
#undef DATAPAGE

}

#define LINEAR_HASHMAP_TEMPLATE_PARAM_DECL \
  template <typename Key, typename T, typename Hash, typename Pred>
#define LINEAR_HASHMAP LinearHashMap<Key, T, Hash, Pred>

LINEAR_HASHMAP_TEMPLATE_PARAM_DECL
LINEAR_HASHMAP::LinearHashMap():
  mDataPages(1 << sInitialLevel),
  mNext(0),
  mLevel(sInitialLevel),
  mSize(0)
{
}
LINEAR_HASHMAP_TEMPLATE_PARAM_DECL
typename LINEAR_HASHMAP::iterator LINEAR_HASHMAP::begin() noexcept
{
  if(empty())
  {
    return end();
  }
  auto it = std::find_if(mDataPages.begin(), mDataPages.end(), [](const DataPage& page){ return !page.empty(); });
  return iterator(this, it - mDataPages.begin(), 0);
}
LINEAR_HASHMAP_TEMPLATE_PARAM_DECL
typename LINEAR_HASHMAP::iterator LINEAR_HASHMAP::end() noexcept
{
  return iterator{};
}
LINEAR_HASHMAP_TEMPLATE_PARAM_DECL
typename LINEAR_HASHMAP::const_iterator LINEAR_HASHMAP::begin() const noexcept
{
  if(empty())
  {
    return end();
  }
  auto it = std::find_if(mDataPages.begin(), mDataPages.end(), [](const DataPage& page){ return !page.empty(); });
  return const_iterator(this, it - mDataPages.begin(), 0);
}
LINEAR_HASHMAP_TEMPLATE_PARAM_DECL
typename LINEAR_HASHMAP::const_iterator LINEAR_HASHMAP::end() const noexcept
{
  return const_iterator();
}

LINEAR_HASHMAP_TEMPLATE_PARAM_DECL
std::size_t LINEAR_HASHMAP::makeMask()
{
  std::size_t ans = 0;
  for(std::size_t i = 0; i < mLevel; ++i)
  {
    ans <<= 1;
    ans |= 1;
  }
  return ans;
}
LINEAR_HASHMAP_TEMPLATE_PARAM_DECL
std::size_t LINEAR_HASHMAP::makePageIndex(const key_type& k)
{
  auto h = mHasher(k);
  auto mask = makeMask();
  auto index = h & mask;
  if(index < mNext)
  {
    index = (h & (mask | 1 << mLevel));
  }
  return index;
}
LINEAR_HASHMAP_TEMPLATE_PARAM_DECL
typename LINEAR_HASHMAP::size_type LINEAR_HASHMAP::size() const noexcept
{
  return mSize;
}
LINEAR_HASHMAP_TEMPLATE_PARAM_DECL
bool LINEAR_HASHMAP::empty() const noexcept
{
  return mSize == 0;
}
LINEAR_HASHMAP_TEMPLATE_PARAM_DECL
template <typename U>
std::pair<typename LINEAR_HASHMAP::iterator, bool> LINEAR_HASHMAP::insertImpl(U&& v)
{
  static_assert(std::is_same_v<std::decay_t<U>, value_type>);
  auto index = makePageIndex(v.first);
  auto& page = mDataPages[index];
  Pred pred;
  auto it = std::find_if(page.begin(), page.end(), [pred, &v](const value_type& val){ return pred(v.first, val.first); });
  if(it != page.end())
  {
    return {iterator(this, index, it.getCurrent()), false};
  }
  page.push_back(std::forward<U>(v));
  ++mSize;
  auto ans = std::make_pair(iterator(this, index, page.size() - 1), true);
  if(sDataPageSize < page.size())
  {
    std::size_t i = mNext;
    mDataPages.emplace_back();
    auto& page1 = mDataPages[i];
    auto& page2 = mDataPages.back();
    assert((i | (1 << mLevel)) == (mDataPages.size() - 1));
    auto mask = makeMask();
    auto newMask = mask | (1 << mLevel);
    for(std::size_t j = page1.size(); 0 < j; --j)
    {
      auto& tmp = page1[j - 1];
      auto h = mHasher(tmp.first);
      if((h & newMask) != i)
      {
        page2.push_back(std::move(tmp));
        page1.remove(j - 1);
      }
    }
    mNext = (mNext + 1) % (1 << mLevel);
    if(mNext == 0)
    {
      ++mLevel;
    }
  }
  return ans;
}
LINEAR_HASHMAP_TEMPLATE_PARAM_DECL
std::pair<typename LINEAR_HASHMAP::iterator, bool> LINEAR_HASHMAP::insert(const value_type& v)
{
  return insertImpl(v);
}
LINEAR_HASHMAP_TEMPLATE_PARAM_DECL
std::pair<typename LINEAR_HASHMAP::iterator, bool> LINEAR_HASHMAP::insert(value_type&& v)
{
  return insertImpl(std::move(v));
}
LINEAR_HASHMAP_TEMPLATE_PARAM_DECL
typename LINEAR_HASHMAP::size_type LINEAR_HASHMAP::erase(const key_type& k)
{
  auto index = makePageIndex(k);
  auto& page = mDataPages[index];
  Pred pred;
  auto it = std::find_if(page.begin(), page.end(), [pred, &k](const value_type& v){ return pred(k, v.first); });
  if(it == page.end())
  {
    return 0;
  }
  page.remove(it);
  if(page.empty() && index == mNext)
  {
  }
  --mSize;
  return 1;
}
LINEAR_HASHMAP_TEMPLATE_PARAM_DECL
typename LINEAR_HASHMAP::iterator LINEAR_HASHMAP::find(const key_type& k)
{
  auto index = makePageIndex(k);
  auto& page = mDataPages[index];
  Pred pred;
  auto it = std::find_if(page.begin(), page.end(), [pred, &k](const value_type& v){ return pred(k, v.first); });
  return it != page.end() ? iterator(this, index, it.getCurrent()) : end();
}
LINEAR_HASHMAP_TEMPLATE_PARAM_DECL
typename LINEAR_HASHMAP::const_iterator LINEAR_HASHMAP::find(const key_type& k) const
{
  auto index = makePageIndex(k);
  auto& page = mDataPages[index];
  Pred pred;
  auto it = std::find_if(page.begin(), page.end(), [pred, &k](const value_type& v){ return pred(k, v.first); });
  return it != page.end() ? const_iterator(this, index, it.getCurrent()) : end();
}

LINEAR_HASHMAP_TEMPLATE_PARAM_DECL
LINEAR_HASHMAP::Iterator::Iterator() noexcept:
  mThis(nullptr),
  mPageIndex(0),
  mCurrent(0)
{
}
LINEAR_HASHMAP_TEMPLATE_PARAM_DECL
LINEAR_HASHMAP::Iterator::Iterator(ThisType* self, std::size_t pageIndex, std::size_t current) noexcept:
  mThis(self),
  mPageIndex(pageIndex),
  mCurrent(current)
{
}
LINEAR_HASHMAP_TEMPLATE_PARAM_DECL
typename LINEAR_HASHMAP::Iterator::reference LINEAR_HASHMAP::Iterator::operator*() noexcept
{
  return mThis->mDataPages[mPageIndex][mCurrent];
}
LINEAR_HASHMAP_TEMPLATE_PARAM_DECL
typename LINEAR_HASHMAP::Iterator::pointer LINEAR_HASHMAP::Iterator::operator->() noexcept
{
  return &mThis->mDataPages[mPageIndex][mCurrent];
}
LINEAR_HASHMAP_TEMPLATE_PARAM_DECL
typename LINEAR_HASHMAP::Iterator& LINEAR_HASHMAP::Iterator::operator++() noexcept
{
  assert(mPageIndex < mThis->mDataPages.size());
  assert(mCurrent < mThis->mDataPages[mPageIndex].size());
  auto& page = mThis->mDataPages[mPageIndex];
  ++mCurrent;
  if(mCurrent < page.size())
  {
    return *this;
  }
  mCurrent = 0;
  ++mPageIndex;
  for(; mPageIndex < mThis->mDataPages.size(); ++mPageIndex)
  {
    if(!mThis->mDataPages[mPageIndex].empty())
    {
      return *this;
    }
  }
  mThis = nullptr;
  mCurrent = 0;
  mPageIndex = 0;
  return *this;
}
LINEAR_HASHMAP_TEMPLATE_PARAM_DECL
typename LINEAR_HASHMAP::Iterator LINEAR_HASHMAP::Iterator::operator++(int) noexcept
{
  auto res = *this;
  ++(*this);
  return res;
}

LINEAR_HASHMAP_TEMPLATE_PARAM_DECL
LINEAR_HASHMAP::ConstIterator::ConstIterator() noexcept:
  mThis(nullptr),
  mPageIndex(0),
  mCurrent(0)
{
}
LINEAR_HASHMAP_TEMPLATE_PARAM_DECL
LINEAR_HASHMAP::ConstIterator::ConstIterator(ThisType* self, std::size_t pageIndex, std::size_t current) noexcept:
  mThis(self),
  mPageIndex(pageIndex),
  mCurrent(current)
{
}
LINEAR_HASHMAP_TEMPLATE_PARAM_DECL
typename LINEAR_HASHMAP::ConstIterator::reference LINEAR_HASHMAP::ConstIterator::operator*() noexcept
{
  return mThis->mDataPages[mPageIndex][mCurrent];
}
LINEAR_HASHMAP_TEMPLATE_PARAM_DECL
typename LINEAR_HASHMAP::ConstIterator::pointer LINEAR_HASHMAP::ConstIterator::operator->() noexcept
{
  return &mThis->mDataPages[mPageIndex][mCurrent];
}
LINEAR_HASHMAP_TEMPLATE_PARAM_DECL
typename LINEAR_HASHMAP::ConstIterator& LINEAR_HASHMAP::ConstIterator::operator++() noexcept
{
  auto& page = mThis->mDataPages[mPageIndex];
  ++mCurrent;
  if(mCurrent != page.size())
  {
    return *this;
  }
  ++mPageIndex;
  if(mPageIndex == mThis->mDataPages.size())
  {
    mThis = nullptr;
    mCurrent = 0;
    mPageIndex = 0;
  }
  return *this;
}
LINEAR_HASHMAP_TEMPLATE_PARAM_DECL
typename LINEAR_HASHMAP::ConstIterator LINEAR_HASHMAP::ConstIterator::operator++(int) noexcept
{
  auto res = *this;
  ++(*this);
  return res;
}

#undef LINEAR_HASHMAP_TEMPLATE_PARAM_DECL
#undef LINEAR_HASHMAP

}
}
