#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>
#include <iostream>
#include <map>
#include <ostream>
#include <random>
#include <string>
#include <unordered_map>
#include "SplitOrderedHashMap/SplitOrderedHashMap.h"

namespace boost
{
namespace test_tools
{
namespace tt_detail
{
template <typename T, typename U>
struct print_log_value<std::pair<T, U>>
{
  void operator()(std::ostream& stream, const std::pair<T, U>& p)
  {
    stream << "[" << p.first << ", " << p.second << "]";
  }
};
}  // namespace tt_detail
}  // namespace test_tools
}  // namespace boost

template <typename Key, typename Value, typename Hasher>
std::map<Key, Value> to_std_map(My::SplitOrderedHashMap::SplitOrderedHashMap<Key, Value, Hasher>& m)
{
  std::map<Key, Value> ans;
  for(auto it = m.begin(); it != m.end(); ++it)
  {
    auto& p = *it;
    auto res = ans.insert(p);
    if(!res.second)
      throw std::runtime_error("aaa");
    BOOST_CHECK(res.second);
  }
  return ans;
}

BOOST_AUTO_TEST_CASE(TestIntegerSplitOrderedHashMap)
{
  constexpr std::size_t elementNum = 1 << 14;
  constexpr std::size_t d = elementNum / 4;
  std::random_device seedGen;
  std::mt19937 engine(seedGen());
  std::uniform_int_distribution<long long> dist;
  My::SplitOrderedHashMap::SplitOrderedHashMap<long long, long long> m1;
  std::map<long long, long long> m2;
  std::vector<std::pair<long long, long long>> v;
  for(std::size_t i = 0; i < elementNum; ++i)
  {
    auto num = dist(engine);
    m1.insert({num, num});
    m2.insert({num, num});
    v.push_back({num, num});
  }
  {
    std::sort(v.begin(), v.end());
    auto it = std::unique(v.begin(), v.end());
    v.erase(it, v.end());

    std::uniform_int_distribution<> indexDist(0, v.size() - 1);
    for(std::size_t i = 0; i < 100; ++i)
    {
      auto j = indexDist(engine);
      auto it = m1.find(v[j].first);
      BOOST_CHECK(it != m1.end());
      BOOST_CHECK_EQUAL(it->first, v[j].first);
      BOOST_CHECK_EQUAL(it->second, v[j].second);
    }
    for(std::size_t i = 0; i < 100; ++i)
    {
      auto k = dist(engine);
      auto it1 = m1.find(k);
      auto it2 = m2.find(k);
      if(it2 == m2.end())
      {
        BOOST_CHECK(it1 == m1.end());
      }
      else
      {
        BOOST_CHECK(it1 != m1.end());
        BOOST_CHECK_EQUAL(it1->first, it2->first);
        BOOST_CHECK_EQUAL(it1->second, it2->second);
      }
    }
    BOOST_CHECK_EQUAL(m1.size(), m2.size());
    auto m3 = to_std_map(m1);
    BOOST_CHECK_EQUAL_COLLECTIONS(m3.begin(), m3.end(), m2.begin(), m2.end());
  }
  for(auto it = v.rbegin(); it != v.rbegin() + d; ++it)
  {
    m1.erase(it->first);
    m2.erase(it->first);
  }
  v.resize(v.size() - d);
  {
    std::uniform_int_distribution<> indexDist(0, v.size() - 1);
    for(std::size_t i = 0; i < 100; ++i)
    {
      auto j = indexDist(engine);
      auto it = m1.find(v[j].first);
      BOOST_CHECK(it != m1.end());
      BOOST_CHECK_EQUAL(it->first, v[j].first);
      BOOST_CHECK_EQUAL(it->second, v[j].second);
    }
    for(std::size_t i = 0; i < 100; ++i)
    {
      auto k = dist(engine);
      auto it1 = m1.find(k);
      auto it2 = m2.find(k);
      if(it2 == m2.end())
      {
        BOOST_CHECK(it1 == m1.end());
      }
      else
      {
        BOOST_CHECK(it1 != m1.end());
        BOOST_CHECK_EQUAL(it1->first, it2->first);
        BOOST_CHECK_EQUAL(it1->second, it2->second);
      }
    }
    BOOST_CHECK_EQUAL(m1.size(), m2.size());
    auto m3 = to_std_map(m1);
    BOOST_CHECK_EQUAL_COLLECTIONS(m3.begin(), m3.end(), m2.begin(), m2.end());
  }
  for(std::size_t i = 0; i < d; ++i)
  {
    auto num = dist(engine);
    m1.insert({num, num});
    m2.insert({num, num});
    v.push_back({num, num});
  }
  {
    std::sort(v.begin(), v.end());
    auto it = std::unique(v.begin(), v.end());
    v.erase(it, v.end());

    std::uniform_int_distribution<> indexDist(0, v.size() - 1);
    for(std::size_t i = 0; i < 100; ++i)
    {
      auto j = indexDist(engine);
      auto it = m1.find(v[j].first);
      BOOST_CHECK(it != m1.end());
      BOOST_CHECK_EQUAL(it->first, v[j].first);
      BOOST_CHECK_EQUAL(it->second, v[j].second);
    }
    for(std::size_t i = 0; i < 100; ++i)
    {
      auto k = dist(engine);
      auto it1 = m1.find(k);
      auto it2 = m2.find(k);
      if(it2 == m2.end())
      {
        BOOST_CHECK(it1 == m1.end());
      }
      else
      {
        BOOST_CHECK(it1 != m1.end());
        BOOST_CHECK_EQUAL(it1->first, it2->first);
        BOOST_CHECK_EQUAL(it1->second, it2->second);
      }
    }
    BOOST_CHECK_EQUAL(m1.size(), m2.size());
    auto m3 = to_std_map(m1);
    BOOST_CHECK_EQUAL_COLLECTIONS(m3.begin(), m3.end(), m2.begin(), m2.end());
  }
  for(auto it = v.rbegin(); it != v.rbegin() + d; ++it)
  {
    m1.erase(it->first);
    m2.erase(it->first);
  }
  v.resize(v.size() - d);
  {
    std::uniform_int_distribution<> indexDist(0, v.size() - 1);
    for(std::size_t i = 0; i < 100; ++i)
    {
      auto j = indexDist(engine);
      auto it = m1.find(v[j].first);
      BOOST_CHECK(it != m1.end());
      BOOST_CHECK_EQUAL(it->first, v[j].first);
      BOOST_CHECK_EQUAL(it->second, v[j].second);
    }
    for(std::size_t i = 0; i < 100; ++i)
    {
      auto k = dist(engine);
      auto it1 = m1.find(k);
      auto it2 = m2.find(k);
      if(it2 == m2.end())
      {
        BOOST_CHECK(it1 == m1.end());
      }
      else
      {
        BOOST_CHECK(it1 != m1.end());
        BOOST_CHECK_EQUAL(it1->first, it2->first);
        BOOST_CHECK_EQUAL(it1->second, it2->second);
      }
    }
    BOOST_CHECK_EQUAL(m1.size(), m2.size());
    auto m3 = to_std_map(m1);
    BOOST_CHECK_EQUAL_COLLECTIONS(m3.begin(), m3.end(), m2.begin(), m2.end());
  }
  for(auto it = v.begin(); it != v.end(); ++it)
  {
    m1.erase(it->first);
    m2.erase(it->first);
  }
  BOOST_CHECK(m1.empty());
  BOOST_CHECK(m1.find(dist(engine)) == m1.end());
}

BOOST_AUTO_TEST_CASE(TestStringSplitOrderedHashMap)
{
  constexpr std::size_t elementNum = 1 << 14;
  constexpr std::size_t d = elementNum / 4;
  std::random_device seedGen;
  std::mt19937 engine(seedGen());
  auto randomString = [&engine]() {
    static std::string chars =
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "1234567890"
        "!@#$%^&*()_+|~-=`[]{};':,./<>?";
    static std::uniform_int_distribution<> dist(0, chars.size() - 1);
    std::size_t len = dist(engine) + 1;
    std::string ans;
    for(std::size_t i = 0; i < len; ++i)
    {
      ans.push_back(chars[dist(engine)]);
    }
    return ans;
  };
  My::SplitOrderedHashMap::SplitOrderedHashMap<std::string, std::string> m1;
  std::map<std::string, std::string> m2;
  std::vector<std::pair<std::string, std::string>> v;
  for(std::size_t i = 0; i < elementNum; ++i)
  {
    auto s = randomString();
    m1.insert({s, s});
    m2.insert({s, s});
    v.push_back({s, s});
  }
  {
    std::sort(v.begin(), v.end());
    auto it = std::unique(v.begin(), v.end());
    v.erase(it, v.end());

    std::uniform_int_distribution<> indexDist(0, v.size() - 1);
    for(std::size_t i = 0; i < 100; ++i)
    {
      auto j = indexDist(engine);
      auto it = m1.find(v[j].first);
      BOOST_CHECK(it != m1.end());
      BOOST_CHECK_EQUAL(it->first, v[j].first);
      BOOST_CHECK_EQUAL(it->second, v[j].second);
    }
    for(std::size_t i = 0; i < 100; ++i)
    {
      auto k = randomString();
      auto it1 = m1.find(k);
      auto it2 = m2.find(k);
      if(it2 == m2.end())
      {
        BOOST_CHECK(it1 == m1.end());
      }
      else
      {
        BOOST_CHECK(it1 != m1.end());
        BOOST_CHECK_EQUAL(it1->first, it2->first);
        BOOST_CHECK_EQUAL(it1->second, it2->second);
      }
    }
    BOOST_CHECK_EQUAL(m1.size(), m2.size());
    auto m3 = to_std_map(m1);
    BOOST_CHECK_EQUAL_COLLECTIONS(m3.begin(), m3.end(), m2.begin(), m2.end());
  }
  for(auto it = v.rbegin(); it != v.rbegin() + d; ++it)
  {
    m1.erase(it->first);
    m2.erase(it->first);
  }
  v.resize(v.size() - d);
  {
    std::uniform_int_distribution<> indexDist(0, v.size() - 1);
    for(std::size_t i = 0; i < 100; ++i)
    {
      auto j = indexDist(engine);
      auto it = m1.find(v[j].first);
      BOOST_CHECK(it != m1.end());
      BOOST_CHECK_EQUAL(it->first, v[j].first);
      BOOST_CHECK_EQUAL(it->second, v[j].second);
    }
    for(std::size_t i = 0; i < 100; ++i)
    {
      auto k = randomString();
      auto it1 = m1.find(k);
      auto it2 = m2.find(k);
      if(it2 == m2.end())
      {
        BOOST_CHECK(it1 == m1.end());
      }
      else
      {
        BOOST_CHECK(it1 != m1.end());
        BOOST_CHECK_EQUAL(it1->first, it2->first);
        BOOST_CHECK_EQUAL(it1->second, it2->second);
      }
    }
    BOOST_CHECK_EQUAL(m1.size(), m2.size());
    auto m3 = to_std_map(m1);
    BOOST_CHECK_EQUAL_COLLECTIONS(m3.begin(), m3.end(), m2.begin(), m2.end());
  }
  for(std::size_t i = 0; i < d; ++i)
  {
    auto s = randomString();
    m1.insert({s, s});
    m2.insert({s, s});
    v.push_back({s, s});
  }
  {
    std::sort(v.begin(), v.end());
    auto it = std::unique(v.begin(), v.end());
    v.erase(it, v.end());

    std::uniform_int_distribution<> indexDist(0, v.size() - 1);
    for(std::size_t i = 0; i < 100; ++i)
    {
      auto j = indexDist(engine);
      auto it = m1.find(v[j].first);
      BOOST_CHECK(it != m1.end());
      BOOST_CHECK_EQUAL(it->first, v[j].first);
      BOOST_CHECK_EQUAL(it->second, v[j].second);
    }
    for(std::size_t i = 0; i < 100; ++i)
    {
      auto k = randomString();
      auto it1 = m1.find(k);
      auto it2 = m2.find(k);
      if(it2 == m2.end())
      {
        BOOST_CHECK(it1 == m1.end());
      }
      else
      {
        BOOST_CHECK(it1 != m1.end());
        BOOST_CHECK_EQUAL(it1->first, it2->first);
        BOOST_CHECK_EQUAL(it1->second, it2->second);
      }
    }
    BOOST_CHECK_EQUAL(m1.size(), m2.size());
    auto m3 = to_std_map(m1);
    BOOST_CHECK_EQUAL_COLLECTIONS(m3.begin(), m3.end(), m2.begin(), m2.end());
  }
  for(auto it = v.rbegin(); it != v.rbegin() + d; ++it)
  {
    m1.erase(it->first);
    m2.erase(it->first);
  }
  v.resize(v.size() - d);
  {
    std::uniform_int_distribution<> indexDist(0, v.size() - 1);
    for(std::size_t i = 0; i < 100; ++i)
    {
      auto j = indexDist(engine);
      auto it = m1.find(v[j].first);
      BOOST_CHECK(it != m1.end());
      BOOST_CHECK_EQUAL(it->first, v[j].first);
      BOOST_CHECK_EQUAL(it->second, v[j].second);
    }
    for(std::size_t i = 0; i < 100; ++i)
    {
      auto k = randomString();
      auto it1 = m1.find(k);
      auto it2 = m2.find(k);
      if(it2 == m2.end())
      {
        BOOST_CHECK(it1 == m1.end());
      }
      else
      {
        BOOST_CHECK(it1 != m1.end());
        BOOST_CHECK_EQUAL(it1->first, it2->first);
        BOOST_CHECK_EQUAL(it1->second, it2->second);
      }
    }
    BOOST_CHECK_EQUAL(m1.size(), m2.size());
    auto m3 = to_std_map(m1);
    BOOST_CHECK_EQUAL_COLLECTIONS(m3.begin(), m3.end(), m2.begin(), m2.end());
  }
  for(auto it = v.begin(); it != v.end(); ++it)
  {
    m1.erase(it->first);
    m2.erase(it->first);
  }
  BOOST_CHECK(m1.empty());
  BOOST_CHECK(m1.find("foo") == m1.end());
}

BOOST_AUTO_TEST_CASE(TestSplitOrderedHashMapCollision)
{
  constexpr std::size_t elementNum = 64;
  constexpr std::size_t d = elementNum / 4;
  struct Hasher
  {
    std::size_t operator()(std::size_t val)
    {
      return val;
    }
  };
  My::SplitOrderedHashMap::SplitOrderedHashMap<std::size_t, std::size_t> m1;
  std::map<std::size_t, std::size_t> m2;
  std::vector<std::pair<std::size_t, std::size_t>> v;
  for(std::size_t i = 0; i < elementNum; ++i)
  {
    auto val = i << 10;
    m1.insert({val, val});
    m2.insert({val, val});
    v.push_back({val, val});
  }
  {
    BOOST_CHECK_EQUAL(m1.size(), m2.size());
    auto m3 = to_std_map(m1);
    BOOST_CHECK_EQUAL_COLLECTIONS(m3.begin(), m3.end(), m2.begin(), m2.end());
  }
  for(auto it = v.rbegin(); it != v.rbegin() + d; ++it)
  {
    m1.erase(it->first);
    m2.erase(it->first);
  }
  v.resize(v.size() - d);
  {
    BOOST_CHECK_EQUAL(m1.size(), m2.size());
    auto m3 = to_std_map(m1);
    BOOST_CHECK_EQUAL_COLLECTIONS(m3.begin(), m3.end(), m2.begin(), m2.end());
  }
  for(std::size_t i = 0; i < d; ++i)
  {
    auto val = (i << 10) + 1;
    m1.insert({val, val});
    m2.insert({val, val});
    v.push_back({val, val});
  }
  {
    BOOST_CHECK_EQUAL(m1.size(), m2.size());
    auto m3 = to_std_map(m1);
    BOOST_CHECK_EQUAL_COLLECTIONS(m3.begin(), m3.end(), m2.begin(), m2.end());
  }
  for(auto it = v.rbegin(); it != v.rbegin() + 2 * d; ++it)
  {
    m1.erase(it->first);
    m2.erase(it->first);
  }
  v.resize(v.size() - d);
  {
    BOOST_CHECK_EQUAL(m1.size(), m2.size());
    auto m3 = to_std_map(m1);
    BOOST_CHECK_EQUAL_COLLECTIONS(m3.begin(), m3.end(), m2.begin(), m2.end());
  }
  for(auto it = v.begin(); it != v.end(); ++it)
  {
    m1.erase(it->first);
    m2.erase(it->first);
  }
  BOOST_CHECK(m1.empty());
  BOOST_CHECK(m1.find(0) == m1.end());
}

