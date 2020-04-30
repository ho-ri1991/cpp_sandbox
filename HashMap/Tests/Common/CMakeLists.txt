cmake_minimum_required(VERSION 3.8)

ADD_EXECUTABLE(test_datapage
  TestDataPage.cpp
)

TARGET_LINK_LIBRARIES(
  test_datapage
  boost_unit_test_framework
  Common
)

ADD_TEST(
  NAME TestDataPage
  COMMAND $<TARGET_FILE:test_datapage> --log_level=message
)

