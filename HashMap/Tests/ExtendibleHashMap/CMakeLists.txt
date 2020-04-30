cmake_minimum_required(VERSION 3.8)

ADD_EXECUTABLE(test_extendible_hashmap
  TestExtendibleHashMap.cpp
)

TARGET_LINK_LIBRARIES(
  test_extendible_hashmap
  boost_unit_test_framework
  ExtendibleHashMap
)

ADD_TEST(
  NAME TestExtendibleHashMap
  COMMAND $<TARGET_FILE:test_extendible_hashmap> --log_level=message
)

