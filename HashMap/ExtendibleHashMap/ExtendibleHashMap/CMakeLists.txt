cmake_minimum_required(VERSION 3.8)

ADD_LIBRARY(ExtendibleHashMap
  INTERFACE
)

TARGET_INCLUDE_DIRECTORIES(ExtendibleHashMap
  INTERFACE
  ../
)

TARGET_LINK_LIBRARIES(ExtendibleHashMap
  INTERFACE
  Common
)

