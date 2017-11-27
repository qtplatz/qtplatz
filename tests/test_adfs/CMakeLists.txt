cmake_minimum_required (VERSION 2.8.9)
cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0042 NEW )

project( test_adfs )
include( "../../qtplatz.cmake" )

#find_package( Boost REQUIRED system filesystem serialization date_time )
find_package( Threads REQUIRED )

include_directories(
  ${Boost_INCLUDE_DIRS}
  ${QTPLATZ_SOURCE_DIR}/src/libs
  )

add_executable( ${PROJECT_NAME} main.cpp )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC ${Boost_LIBRARIES} adfs adportable adcontrols Threads::Threads ${CMAKE_DL_LIBS} )


