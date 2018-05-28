cmake_minimum_required (VERSION 2.8.9)
cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0042 NEW )

project( test_dataprocessor )

include_directories(
  ${Boost_INCLUDE_DIRS}
  ${QTPLATZ_SOURCE_DIR}/src/libs
  )

add_executable( ${PROJECT_NAME}
  main.cpp
  dataprocessor.hpp
  dataprocessor.cpp
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adfs
  adportable
  adcontrols
  adprocessor
  ${Boost_LIBRARIES}
  )


