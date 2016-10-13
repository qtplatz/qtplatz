cmake_minimum_required( VERSION 3.5 )
project( counting )
cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0020 NEW)
cmake_policy(SET CMP0028 NEW)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set ( CMAKE_CXX_STANDARD 14 )

# include( "standalone.cmake" )
if ( NOT Boost_FOUND )
  find_package( Boost COMPONENTS filesystem system program_options serialization wserialization )
endif()

if ( OpenCV_FOUND )
  add_definitions( "-DOPENCV=1" )
  include_directories( ${OpenCV_INCLUDE_DIRS} ${HDF5_INCLUDE_DIRS} )
  set( _opencv_libraries ${OpenCV_LIBRARIES} ${HDF5_LIBRARIES} )
endif()

include_directories( ${QTPLATZ_INCLUDE_DIRS} ${QWT_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/src )

add_executable( ${PROJECT_NAME}
  main.cpp
  rawdata.cpp
  rawdata.hpp
  ${QTPLATZ_SOURCE_DIR}/src/adplugins/adtextfile/time_data_reader.cpp
  ${QTPLATZ_SOURCE_DIR}/src/adplugins/adtextfile/time_data_reader.hpp
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adportable
  adcontrols
  adfs
  ${Boost_LIBRARIES}
  ${_opencv_libraries}
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT libraries 
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries
  )
