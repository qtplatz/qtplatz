cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0042 NEW )

project( test_pkarea )

include_directories(
  ${Qt5Core_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  )

add_executable( ${PROJECT_NAME}
  main.cpp
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  ${Boost_LIBRARIES}
  Qt5::Core
  adportable
  )

#set_target_properties( ${PROJECT_NAME} PROPERTIES CXX_STANDARD 17 )
#target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC ${Boost_LIBRARIES} ${CMAKE_DL_LIBS} )
