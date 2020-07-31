#include <type_traits>

namespace My
{
namespace SplitOrderedHashMap
{

#define SPLITORDERED_HASHMAP_TEMPLATE_PARAM_DECL \
  template <typename Key, typename T, typename Hash, typename Pred>
#define SPLITORDERED_HASHMAP SplitOrderedHashMap<Key, T, Hash, Pred>

SPLITORDERED_HASHMAP_TEMPLATE_PARAM_DECL
SPLITORDERED_HASHMAP::ListElementType::ListElementType(const ValueType& v, HashValueType key):
  mKey(key)
{
  assert(!isSentinel());
  new(mData) ValueType(v);
}
SPLITORDERED_HASHMAP_TEMPLATE_PARAM_DECL
SPLITORDERED_HASHMAP::ListElementType::ListElementType(ValueType&& value, HashValueType key) noexcept(std::is_nothrow_move_constructible_v<ValueType>):
  mKey(key)
{
  assert(!isSentinel());
  new(mData) ValueType(std::move(value));
}
SPLITORDERED_HASHMAP_TEMPLATE_PARAM_DECL
SPLITORDERED_HASHMAP::ListElementType::ListElementType(HashValueType k) noexcept:
  mKey(k)
{
  assert(isSentinel());
}
SPLITORDERED_HASHMAP_TEMPLATE_PARAM_DECL
SPLITORDERED_HASHMAP::ListElementType::ListElementType(const ListElementType& other):
  mKey(other.mKey)
{
  if(!other.isSentinel())
  {
    new(mData) ValueType(other.getValue());
  }
}
SPLITORDERED_HASHMAP_TEMPLATE_PARAM_DECL
SPLITORDERED_HASHMAP::ListElementType::ListElementType(ListElementType&& other) noexcept(std::is_nothrow_move_constructible_v<ValueType>):
  mKey(other.mKey)
{
  if(!other.isSentinel())
  {
    new(mData) ValueType(std::move(other.getValue()));
  }
}
SPLITORDERED_HASHMAP_TEMPLATE_PARAM_DECL
SPLITORDERED_HASHMAP::ListElementType::~ListElementType() noexcept
{
  if constexpr(!std::is_trivially_destructible_v<ValueType>)
  {
    if(!isSentinel())
    {
      reinterpret_cast<ValueType*>(mData)->~ValueType();
    }
  }
}
SPLITORDERED_HASHMAP_TEMPLATE_PARAM_DECL
typename SPLITORDERED_HASHMAP::ListElementType::ValueType& SPLITORDERED_HASHMAP::ListElementType::getValue() noexcept
{
  assert(!isSentinel());
  return *std::launder(reinterpret_cast<ValueType*>(mData));
}
SPLITORDERED_HASHMAP_TEMPLATE_PARAM_DECL
const typename SPLITORDERED_HASHMAP::ListElementType::ValueType& SPLITORDERED_HASHMAP::ListElementType::getValue() const noexcept
{
  assert(!isSentinel());
  return *std::launder(reinterpret_cast<const ValueType*>(mData));
}
SPLITORDERED_HASHMAP_TEMPLATE_PARAM_DECL
typename SPLITORDERED_HASHMAP::HashValueType SPLITORDERED_HASHMAP::ListElementType::getKey() const noexcept
{
  return mKey;
}
SPLITORDERED_HASHMAP_TEMPLATE_PARAM_DECL
bool SPLITORDERED_HASHMAP::ListElementType::isSentinel() const noexcept
{
  return (mKey & 1) == 0;
}

SPLITORDERED_HASHMAP_TEMPLATE_PARAM_DECL
constexpr typename SPLITORDERED_HASHMAP::HashValueType SPLITORDERED_HASHMAP::reverse(HashValueType h)
{
  HashValueType ans = 0;
  for(std::size_t i = 0; i < sizeof(HashValueType) * 8; ++i)
  {
    ans <<= 1;
    ans |= (h & 1);
    h >>= 1;
  }
  return ans;
}
SPLITORDERED_HASHMAP_TEMPLATE_PARAM_DECL
constexpr typename SPLITORDERED_HASHMAP::HashValueType SPLITORDERED_HASHMAP::makeSentinelKey(HashValueType h)
{
  return reverse(h & sMask);
}
SPLITORDERED_HASHMAP_TEMPLATE_PARAM_DECL
typename SPLITORDERED_HASHMAP::HashValueType SPLITORDERED_HASHMAP::makeOrdinaryKey(const key_type& k)
{
  auto h = mHash(k) & sMask;
  return reverse(h | sHiMask);
}
SPLITORDERED_HASHMAP_TEMPLATE_PARAM_DECL
typename SPLITORDERED_HASHMAP::HashValueType SPLITORDERED_HASHMAP::makeBaseArrayMask() const noexcept
{
  HashValueType ans = 0;
  auto tmp = mBaseArray.size() / 2;
  while(tmp)
  {
    ans <<= 1;
    ans |= 1;
    tmp >>= 1;
  }
  return ans;
}
SPLITORDERED_HASHMAP_TEMPLATE_PARAM_DECL
typename SPLITORDERED_HASHMAP::HashValueType SPLITORDERED_HASHMAP::prevBaseArrayIndex(HashValueType index) const noexcept
{
  assert(index < mBaseArray.size());
  HashValueType splitOrderKey = 0;
  for(std::size_t i = 0; i < mLevel; ++i)
  {
    splitOrderKey <<= 1;
    splitOrderKey |= index & 1;
    index >>= 1;
  }
  --splitOrderKey;
  HashValueType ans = 0;
  for(std::size_t i = 0; i < mLevel; ++i)
  {
    ans <<= 1;
    ans |= splitOrderKey & 1;
    splitOrderKey >>= 1;
  }
  return ans;
}
SPLITORDERED_HASHMAP_TEMPLATE_PARAM_DECL
typename SPLITORDERED_HASHMAP::ListType::iterator SPLITORDERED_HASHMAP::begin(HashValueType index)
{
  assert(index < mBaseArray.size());
  auto listIt = mBaseArray[index];
  if(listIt == mSplitOrderedList.end())
  {
    assert(0 < index);
    auto it = begin(prevBaseArrayIndex(index));
    auto sentinelKey = makeSentinelKey(index);
    it = std::find_if(it, mSplitOrderedList.end(), [sentinelKey](const ListElementType& elem){ return sentinelKey < elem.getKey(); });
    listIt = mSplitOrderedList.insert(it, ListElementType(sentinelKey));
    mBaseArray[index] = listIt;
  }
  assert(listIt != mSplitOrderedList.end());
  assert(listIt->isSentinel());
  return listIt;
}
SPLITORDERED_HASHMAP_TEMPLATE_PARAM_DECL
typename SPLITORDERED_HASHMAP::ListType::iterator SPLITORDERED_HASHMAP::begin(const key_type& k)
{
  auto h = mHash(k);
  auto index = h & makeBaseArrayMask();
  return begin(index);
}
SPLITORDERED_HASHMAP_TEMPLATE_PARAM_DECL
template <typename U>
std::pair<typename SPLITORDERED_HASHMAP::iterator, bool> SPLITORDERED_HASHMAP::insertImpl(U&& v)
{
  static_assert(std::is_same_v<std::decay_t<U>, value_type>);
  auto listIt = begin(v.first);
  auto k = makeOrdinaryKey(v.first);
  Pred pred;
  auto it = std::find_if(listIt, mSplitOrderedList.end(), [k, &v, pred](const ListElementType& elem){ return k < elem.getKey() || (!elem.isSentinel() && pred(elem.getValue().first, v.first)); });
  if(it != mSplitOrderedList.end() && !it->isSentinel() && pred(it->getValue().first, v.first))
  {
    return {iterator(it, mSplitOrderedList.end()), false};
  }
  auto res = mSplitOrderedList.insert(it, ListElementType(std::forward<U>(v), k));
  ++mSize;
  if(sThreshold <= mSize / mBaseArray.size())
  {
    mBaseArray.resize(mBaseArray.size() * 2, mSplitOrderedList.end());
    ++mLevel;
  }
  return {iterator(res, mSplitOrderedList.end()), true};
}

SPLITORDERED_HASHMAP_TEMPLATE_PARAM_DECL
SPLITORDERED_HASHMAP::SplitOrderedHashMap():
  mSplitOrderedList({ListElementType(makeSentinelKey(0)), ListElementType(makeSentinelKey(1))}),
  mBaseArray(2),
  mSize(0),
  mLevel(1)
{
  auto it = mSplitOrderedList.begin();
  mBaseArray[0] = it;
  mBaseArray[1] = std::next(it);
}
SPLITORDERED_HASHMAP_TEMPLATE_PARAM_DECL
typename SPLITORDERED_HASHMAP::iterator SPLITORDERED_HASHMAP::begin() noexcept
{
  return iterator(std::find_if(mSplitOrderedList.begin(), mSplitOrderedList.end(), [](const ListElementType& elem){ return !elem.isSentinel();}), mSplitOrderedList.end());
}
SPLITORDERED_HASHMAP_TEMPLATE_PARAM_DECL
typename SPLITORDERED_HASHMAP::iterator SPLITORDERED_HASHMAP::end() noexcept
{
  return iterator(mSplitOrderedList.end(), mSplitOrderedList.end());
}
SPLITORDERED_HASHMAP_TEMPLATE_PARAM_DECL
typename SPLITORDERED_HASHMAP::const_iterator SPLITORDERED_HASHMAP::begin() const noexcept
{
  return const_iterator(std::find_if(mSplitOrderedList.begin(), mSplitOrderedList.end(), [](const ListElementType& elem){ return !elem.isSentinel();}), mSplitOrderedList.end());
}
SPLITORDERED_HASHMAP_TEMPLATE_PARAM_DECL
typename SPLITORDERED_HASHMAP::const_iterator SPLITORDERED_HASHMAP::end() const noexcept
{
  return const_iterator(mSplitOrderedList.end(), mSplitOrderedList.end());
}
SPLITORDERED_HASHMAP_TEMPLATE_PARAM_DECL
typename SPLITORDERED_HASHMAP::size_type SPLITORDERED_HASHMAP::size() const noexcept
{
  return mSize;
}
SPLITORDERED_HASHMAP_TEMPLATE_PARAM_DECL
bool SPLITORDERED_HASHMAP::empty() const noexcept
{
  return mSize == 0;
}
SPLITORDERED_HASHMAP_TEMPLATE_PARAM_DECL
std::pair<typename SPLITORDERED_HASHMAP::iterator, bool> SPLITORDERED_HASHMAP::insert(const value_type& v)
{
  return insertImpl(v);
}
SPLITORDERED_HASHMAP_TEMPLATE_PARAM_DECL
std::pair<typename SPLITORDERED_HASHMAP::iterator, bool> SPLITORDERED_HASHMAP::insert(value_type&& v)
{
  return insertImpl(std::move(v));
}
SPLITORDERED_HASHMAP_TEMPLATE_PARAM_DECL
typename SPLITORDERED_HASHMAP::size_type SPLITORDERED_HASHMAP::erase(const key_type& k)
{
  Pred pred;
  auto listKey = makeOrdinaryKey(k);
  auto it = std::find_if(begin(k), mSplitOrderedList.end(), [pred, listKey, &k](const ListElementType& elem){ return listKey < elem.getKey() || (!elem.isSentinel() && pred(k, elem.getValue().first)); });
  if(it == mSplitOrderedList.end() || it->isSentinel() || !pred(k, it->getValue().first))
  {
    return 0;
  }
  mSplitOrderedList.erase(it);
  --mSize;
  return 1;
}
SPLITORDERED_HASHMAP_TEMPLATE_PARAM_DECL
typename SPLITORDERED_HASHMAP::iterator SPLITORDERED_HASHMAP::find(const key_type& k)
{
  Pred pred;
  auto listKey = makeOrdinaryKey(k);
  auto it = std::find_if(begin(k), mSplitOrderedList.end(), [pred, listKey, &k](const ListElementType& elem){ return listKey < elem.getKey() || (!elem.isSentinel() && pred(k, elem.getValue().first)); });
  if(it == mSplitOrderedList.end() || it->isSentinel() || !pred(k, it->getValue().first))
  {
    return end();
  }
  return iterator(it, mSplitOrderedList.end());
}

SPLITORDERED_HASHMAP_TEMPLATE_PARAM_DECL
SPLITORDERED_HASHMAP::Iterator::Iterator(typename SPLITORDERED_HASHMAP::ListType::iterator it, typename SPLITORDERED_HASHMAP::ListType::iterator end) noexcept:
  mListIterator(it),
  mEnd(end)
{
}
SPLITORDERED_HASHMAP_TEMPLATE_PARAM_DECL
typename SPLITORDERED_HASHMAP::Iterator::reference SPLITORDERED_HASHMAP::Iterator::operator*() noexcept
{
  assert(mListIterator != mEnd);
  return mListIterator->getValue();
}
SPLITORDERED_HASHMAP_TEMPLATE_PARAM_DECL
typename SPLITORDERED_HASHMAP::Iterator::pointer SPLITORDERED_HASHMAP::Iterator::operator->() noexcept
{
  assert(mListIterator != mEnd);
  return &mListIterator->getValue();
}
SPLITORDERED_HASHMAP_TEMPLATE_PARAM_DECL
typename SPLITORDERED_HASHMAP::Iterator& SPLITORDERED_HASHMAP::Iterator::operator++() noexcept
{
  assert(mListIterator != mEnd);
  auto it = std::find_if(std::next(mListIterator), mEnd, [](const ListElementType& elem){ return !elem.isSentinel(); });
  mListIterator = it;
  return *this;
}
SPLITORDERED_HASHMAP_TEMPLATE_PARAM_DECL
typename SPLITORDERED_HASHMAP::Iterator SPLITORDERED_HASHMAP::Iterator::operator++(int) noexcept
{
  auto ans = (*this);
  ++(*this);
  return ans;
}

SPLITORDERED_HASHMAP_TEMPLATE_PARAM_DECL
SPLITORDERED_HASHMAP::ConstIterator::ConstIterator(typename ListType::const_iterator it, typename ListType::const_iterator end) noexcept:
  mListIterator(it),
  mEnd(end)
{
}
SPLITORDERED_HASHMAP_TEMPLATE_PARAM_DECL
typename SPLITORDERED_HASHMAP::ConstIterator::reference SPLITORDERED_HASHMAP::ConstIterator::operator*() noexcept
{
  assert(mListIterator != mEnd);
  return mListIterator->getValue();
}
SPLITORDERED_HASHMAP_TEMPLATE_PARAM_DECL
typename SPLITORDERED_HASHMAP::ConstIterator::pointer SPLITORDERED_HASHMAP::ConstIterator::operator->() noexcept
{
  assert(mListIterator != mEnd);
  return &mListIterator->getValue();
}
SPLITORDERED_HASHMAP_TEMPLATE_PARAM_DECL
typename SPLITORDERED_HASHMAP::ConstIterator& SPLITORDERED_HASHMAP::ConstIterator::operator++() noexcept
{
  assert(mListIterator != mEnd);
  auto it = std::find_if(std::next(mListIterator), mEnd, [](const ListElementType& elem){ return !elem.isSentinel(); });
  mListIterator = it;
  return *this;
}
SPLITORDERED_HASHMAP_TEMPLATE_PARAM_DECL
typename SPLITORDERED_HASHMAP::ConstIterator SPLITORDERED_HASHMAP::ConstIterator::operator++(int) noexcept
{
  auto ans = (*this);
  ++(*this);
  return ans;
}

}
}

