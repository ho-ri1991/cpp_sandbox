cmake_minimum_required(VERSION 3.8)

ADD_EXECUTABLE(test_splitordered_hashmap
  TestSplitOrderedHashMap.cpp
)

TARGET_LINK_LIBRARIES(
  test_splitordered_hashmap
  boost_unit_test_framework
  SplitOrderedHashMap 
)

ADD_TEST(
  NAME TestSplitOrderedHashMap
  COMMAND $<TARGET_FILE:test_splitordered_hashmap> --log_level=message
)

