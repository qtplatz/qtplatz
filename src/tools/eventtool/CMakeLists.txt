project( eventtool )
cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0020 NEW)
cmake_policy(SET CMP0028 NEW)

set( eventtool_SOURCES 
  document.cpp
  document.hpp
  main.cpp
  mainwindow.cpp
  mainwindow.hpp
  eventform.cpp
  eventform.hpp
  )

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB UIs *.ui)
file(GLOB Resources *.qrc)

include_directories( ${QWT}/include )
link_directories( ${QWT}/lib )

find_package(Qt5 CONFIG REQUIRED Widgets Gui Svg Sql PrintSupport )

if ( NOT Boost_FOUND )
  #find_package(Boost COMPONENTS system )
endif()

if ( WIN32 )
  add_executable( eventtool WIN32 ${eventtool_SOURCES} ${Resources} )
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4251 /wd4800" )
  set ( libs "" )
else()
  add_executable( eventtool ${eventtool_SOURCES} ${Resources} )
  set ( libs "pthread" )
endif()

target_link_libraries( eventtool LINK_PUBLIC
		       adportable
		       adwidgets
		       acewrapper
		       Qt5::Widgets
		       Qt5::Widgets
		       ${Boost_LIBRARIES} ${libs} )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT libraries 
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )
