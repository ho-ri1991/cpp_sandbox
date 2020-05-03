#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN

#include <algorithm>
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <ostream>
#include <random>
#include <string>
#include "Common/DataPage.h"

BOOST_AUTO_TEST_CASE(TestIntegerDataPage)
{
  My::Common::DataPage<int, 4> page;
  BOOST_CHECK(page.empty());
  BOOST_CHECK(!page.full());
  BOOST_CHECK_EQUAL(page.size(), 0);

  page.push_back(10);
  BOOST_CHECK(!page.empty());
  BOOST_CHECK_EQUAL(page.size(), 1);
  BOOST_CHECK_EQUAL(*std::find(page.begin(), page.end(), 10), 10);

  page.push_back(11);
  page.push_back(12);
  page.push_back(13);
  BOOST_CHECK_EQUAL(*std::find(page.begin(), page.end(), 12), 12);
  BOOST_CHECK(page.full());

  page.remove(0);
  BOOST_CHECK(std::find(page.begin(), page.end(), 10) == page.end());
  BOOST_CHECK_EQUAL(*std::find(page.begin(), page.end(), 11), 11);
  BOOST_CHECK_EQUAL(*std::find(page.begin(), page.end(), 12), 12);
  BOOST_CHECK_EQUAL(*std::find(page.begin(), page.end(), 13), 13);

  auto it = std::find(page.begin(), page.end(), 12);
  BOOST_CHECK(it != page.end());
  page.remove(it - page.begin());
  BOOST_CHECK(std::find(page.begin(), page.end(), 10) == page.end());
  BOOST_CHECK(std::find(page.begin(), page.end(), 12) == page.end());
  BOOST_CHECK_EQUAL(*std::find(page.begin(), page.end(), 11), 11);
  BOOST_CHECK_EQUAL(*std::find(page.begin(), page.end(), 13), 13);

  it = std::find(page.begin(), page.end(), 13);
  BOOST_CHECK(it != page.end());
  page.remove(it - page.begin());
  BOOST_CHECK(std::find(page.begin(), page.end(), 10) == page.end());
  BOOST_CHECK(std::find(page.begin(), page.end(), 12) == page.end());
  BOOST_CHECK_EQUAL(*std::find(page.begin(), page.end(), 11), 11);
  BOOST_CHECK(std::find(page.begin(), page.end(), 13) == page.end());

  it = std::find(page.begin(), page.end(), 11);
  BOOST_CHECK(it != page.end());
  page.remove(it - page.begin());
  BOOST_CHECK(page.empty());

  using std::swap;
  My::Common::DataPage<int, 4> page1, page2;
  page1.push_back(42);
  swap(page1, page2);
  BOOST_CHECK_EQUAL(page1.size(), 0);
  BOOST_CHECK_EQUAL(page2.size(), 1);
  BOOST_CHECK_EQUAL(page2[0], 42);

  page2.push_back(43);
  page2.push_back(44);
  page2.push_back(45);
  swap(page1, page2);
  BOOST_CHECK_EQUAL(page1.size(), 4);
  BOOST_CHECK_EQUAL(page2.size(), 0);
}

BOOST_AUTO_TEST_CASE(TestStringDataPage)
{
  My::Common::DataPage<std::string, 4> page;
  BOOST_CHECK(page.empty());
  BOOST_CHECK(!page.full());
  BOOST_CHECK_EQUAL(page.size(), 0);

  page.push_back("a");
  BOOST_CHECK(!page.empty());
  BOOST_CHECK_EQUAL(page.size(), 1);
  BOOST_CHECK_EQUAL(*std::find(page.begin(), page.end(), "a"), "a");

  page.push_back("b");
  page.push_back("c");
  page.push_back("d");
  BOOST_CHECK_EQUAL(*std::find(page.begin(), page.end(), "c"), "c");
  BOOST_CHECK(page.full());

  page.remove(0);
  BOOST_CHECK(std::find(page.begin(), page.end(), "a") == page.end());
  BOOST_CHECK_EQUAL(*std::find(page.begin(), page.end(), "b"), "b");
  BOOST_CHECK_EQUAL(*std::find(page.begin(), page.end(), "c"), "c");
  BOOST_CHECK_EQUAL(*std::find(page.begin(), page.end(), "d"), "d");

  auto it = std::find(page.begin(), page.end(), "c");
  BOOST_CHECK(it != page.end());
  page.remove(it - page.begin());
  BOOST_CHECK(std::find(page.begin(), page.end(), "a") == page.end());
  BOOST_CHECK(std::find(page.begin(), page.end(), "c") == page.end());
  BOOST_CHECK_EQUAL(*std::find(page.begin(), page.end(), "b"), "b");
  BOOST_CHECK_EQUAL(*std::find(page.begin(), page.end(), "d"), "d");

  it = std::find(page.begin(), page.end(), "d");
  BOOST_CHECK(it != page.end());
  page.remove(it - page.begin());
  BOOST_CHECK(std::find(page.begin(), page.end(), "a") == page.end());
  BOOST_CHECK(std::find(page.begin(), page.end(), "c") == page.end());
  BOOST_CHECK_EQUAL(*std::find(page.begin(), page.end(), "b"), "b");
  BOOST_CHECK(std::find(page.begin(), page.end(), "d") == page.end());

  it = std::find(page.begin(), page.end(), "b");
  BOOST_CHECK(it != page.end());
  page.remove(it - page.begin());
  BOOST_CHECK(page.empty());

  using std::swap;
  My::Common::DataPage<std::string, 4> page1, page2;
  page1.push_back("a");
  swap(page1, page2);
  BOOST_CHECK_EQUAL(page1.size(), 0);
  BOOST_CHECK_EQUAL(page2.size(), 1);
  BOOST_CHECK_EQUAL(page2[0], "a");

  page2.push_back("b");
  page2.push_back("c");
  page2.push_back("d");
  swap(page1, page2);
  BOOST_CHECK_EQUAL(page1.size(), 4);
  BOOST_CHECK_EQUAL(page2.size(), 0);
}
