cmake_policy( SET CMP0020 NEW )
project( addatafile )

find_package( Qt5 CONFIG REQUIRED Core )

configure_file(
  "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.adplugin"
  "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${PROJECT_NAME}.adplugin"
  )

include_directories( ${Qt5Core_INCLUDE_DIRS}  )

add_definitions(-DADDATAFILE_LIBRARY)

#  LIBS += -lboost_system -lboost_filesystem -lboost_serialization -lboost_date_time -ldl

add_library( addatafile MODULE 
  addatafile.cpp
  datafile.cpp
  datafile_factory.cpp
  rawdata.cpp
  addatafile.hpp
  addatafile_global.h
  datafile.hpp
  datafile_factory.hpp
  rawdata.hpp
  )

target_link_libraries( addatafile LINK_PUBLIC
  ${Boost_LIBRARIES}
  adplugin
  acewrapper
  adcontrols
  adlog
  adfs
  adportable
  adutils
  portfolio
  xmlparser
  adplugin
)
        
