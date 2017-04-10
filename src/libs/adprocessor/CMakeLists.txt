
project(adprocessor)

cmake_policy(SET CMP0020 NEW)
cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0028 NEW)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

if ( NOT Boost_FOUND )
  #find_package(Boost 1.57 REQUIRED COMPONENTS filesystem wserialization serialization )
endif()

include_directories(
  ${QWT_INCLUDE_DIR}
  ${Qt5PrintSupport_INCLUDE_DIRS}
  ${Qt5Svg_INCLUDE_DIRS}
  ${Qt5Sql_INCLUDE_DIRS}  
  ${Qt5Widgets_INCLUDE_DIRS}
  )

add_definitions(-DADPROCESSOR_LIBRARY)
if ( MSVC )
  add_definitions(-D_SCL_SECURE_NO_WARNINGS)
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4251" )
endif()

add_library( ${PROJECT_NAME} SHARED
  dataprocessor.hpp
  dataprocessor.cpp
  mschromatogramextractor.hpp
  mschromatogramextractor_accumulate.hpp
  mschromatogramextractor_xchromatogram.hpp
  mschromatogramextractor_v2.cpp
  mschromatogramextractor_v2.hpp
  mschromatogramextractor_v3.cpp
  mschromatogramextractor_v3.hpp       
  )

target_link_libraries( adprocessor LINK_PUBLIC
  adcontrols
  adfs
  portfolio
  ${Boost_LIBRARIES}
  )

set_property( TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14 )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT modules
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )
