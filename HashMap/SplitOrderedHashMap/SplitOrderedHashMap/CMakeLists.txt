cmake_minimum_required(VERSION 3.8)

ADD_LIBRARY(SplitOrderedHashMap
  INTERFACE
)

TARGET_INCLUDE_DIRECTORIES(SplitOrderedHashMap
  INTERFACE
  ../
)

TARGET_LINK_LIBRARIES(SplitOrderedHashMap
  INTERFACE
  Common
)

