cmake_minimum_required(VERSION 3.8)

ADD_LIBRARY(LinearHashMap
  INTERFACE
)

TARGET_INCLUDE_DIRECTORIES(LinearHashMap
  INTERFACE
  ../
)

TARGET_LINK_LIBRARIES(LinearHashMap
  INTERFACE
  Common
)

