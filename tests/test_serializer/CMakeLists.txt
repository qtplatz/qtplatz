cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0042 NEW )

project( test_serializer )

#find_package( Boost REQUIRED COMPONENTS
#  serialization
#  system
#  unit_test_framework
#  wserialization
#  )

include_directories(
  ${Boost_INCLUDE_DIRS}
  ${QTPLATZ_SOURCE_DIR}/src/libs
  )

add_executable( ${PROJECT_NAME}
  test_process_method.cpp
  test_process_method.hpp
  test_massspectrum.cpp
  test_massspectrum.hpp
  main.cpp
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adcontrols
  adportable
  ${Boost_LIBRARIES}
  )
