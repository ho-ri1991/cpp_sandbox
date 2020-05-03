cmake_minimum_required(VERSION 3.8)

ADD_EXECUTABLE(test_linear_hashmap
  TestLinearHashMap.cpp
)

TARGET_LINK_LIBRARIES(
  test_linear_hashmap
  boost_unit_test_framework
  LinearHashMap 
)

ADD_TEST(
  NAME TestLinearHashMap
  COMMAND $<TARGET_FILE:test_linear_hashmap> --log_level=message
)

