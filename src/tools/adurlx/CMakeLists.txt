cmake_minimum_required (VERSION 3.18.2)
cmake_policy( SET CMP0022 NEW )

project( adurlx )

find_package( Threads REQUIRED )
#find_package( BZip2 REQUIRED )

include_directories(
  ${CMAKE_SOURCE_DIR}
  ${QTPLATZ_SOURCE_DIR}/src/libs
  )

add_executable( ${PROJECT_NAME} main.cpp )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adurl
  adio
  adportable
  Threads::Threads
  ${Boost_LIBRARIES}
  #  ${BZIP2_LIBRARIES}
  )

install( TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin COMPONENT applications )
