cmake_policy( SET CMP0020 NEW )

find_package( Qt5 CONFIG REQUIRED Core )

include_directories( ${Qt5Core_INCLUDE_DIRS}  )

add_definitions(-DSHRADER_LRPFILE_LIBRARY)

add_library( shrader_lrpfile SHARED
  shrader_lrpfile.cpp
  datafile_factory.cpp
  datafile_factory.hpp
  datafile.cpp
  datafile.hpp
)

target_link_libraries( shrader_lrpfile LINK_PUBLIC
  ${Boost_LIBRARIES}
  lrpfile  
  adplugin
  adcontrols
  portfolio )

#target_include_directories( sitcp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )
