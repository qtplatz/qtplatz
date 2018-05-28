cmake_minimum_required (VERSION 2.8.9)
cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0042 NEW )

project( test_adfs )

include_directories(
  ${Boost_INCLUDE_DIRS}
  ${QTPLATZ_SOURCE_DIR}/src/libs
  )

add_executable( ${PROJECT_NAME}
  main.cpp
  test_massspectrum.hpp
  test_massspectrum.cpp
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adfs
  adportable
  adcontrols
  ${Boost_LIBRARIES}
  )


