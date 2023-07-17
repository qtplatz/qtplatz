
cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0042 NEW )

project( test_formula )

add_definitions( -Wno-deprecated-declarations )

include_directories(
  ${Boost_INCLUDE_DIRS}
  ${QTPLATZ_SOURCE_DIR}/src/libs
  )

add_executable( ${PROJECT_NAME}
  main.cpp
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adportable
  adcontrols
  ${Boost_LIBRARIES}
  )
