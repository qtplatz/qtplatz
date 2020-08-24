cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0028 NEW )

project( cluster )
set( PROVIDER MS-Cheminformatics )
set( COMPAT_VERSION 3.2.81 )
set( VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )

find_package( Qt5 CONFIG REQUIRED Core Svg Sql PrintSupport Widgets Network OpenGL )
find_package( qwt )

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB UIs *.ui)
file(GLOB Resources *.qrc)

if ( OpenCV_FOUND )
  include_directories( ${OpenCV_INCLUDE_DIRS} )
endif()

find_package( Threads REQUIRED )

configure_file(
  "${PROJECT_SOURCE_DIR}/../pluginspec.in"
  "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec"
  )

execute_process( COMMAND ${XMLPATTERNS} -no-format
  -output ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.json
  ${QTPLATZ_SOURCE_DIR}/src/pluginjsonmetadata.xsl
  ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec
  )

add_definitions(
  -D_SCL_SECURE_NO_WARNINGS
  )
if ( WIN32 )
  add_definitions( "-wd5037" )
endif()

include_directories(
  ${QWT_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${Qt5OpenGL_INCLUDE_DIRS}
  ${Qt5Core_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}
  ${Qt5PrintSupport_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src         # version.h
  ${CMAKE_BINARY_DIR}/src/libs
  ${CMAKE_SOURCE_DIR}/src/libs
  ${CMAKE_SOURCE_DIR}/src/adplugins
  ${QTPLATZ_SOURCE_DIR}/src/plugins
  ${QTPLATZ_SOURCE_DIR}/src      # version number
  )

link_directories(
  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  ${QTPLATZ_BINARY_DIR}/lib/qtplatz  # <== qmake .lib output dir
  ${QTPLATZ_BINARY_DIR}/lib/qtplatz/plubins/QtProject
  )

if ( OpenCV_FOUND )
  set( opencv_SOURCES
    imageview.cpp
    imageview.hpp
    dft2d.cpp
    dft2d.hpp
    cvmat.cpp
    cvmat.hpp
    opencvwnd.hpp
    opencvwnd.cpp
    )
  list( APPEND OPENCV_LIBRARIES advision )
endif()

add_library( ${PROJECT_NAME} MODULE
  clusterplugin.cpp
  clusterplugin.hpp
  contoursform.cpp
  contoursform.hpp
  document.cpp
  document.hpp
  mainwindow.cpp
  mainwindow.hpp
  constants.hpp
  clustereditor.cpp
  clustereditor.hpp
  clusterfactory.cpp
  clusterfactory.hpp
  mode.cpp
  mode.hpp
  player.hpp
  player.cpp
  playerwnd.hpp
  playerwnd.cpp
  uimediator.hpp
  uimediator.cpp
  ${opencv_SOURCES}
  ${Resources}
#  ${malpix_RESOURCES}
  )

if ( MSVC )
  set_source_files_properties( cvmat.cpp PROPERTIES COMPILE_FLAGS -wd4267 ) # size_t to int
endif()

target_link_libraries(
  ${PROJECT_NAME} LINK_PUBLIC
  adplugin
  adcontrols
  adlog
  adutils
  adfs
  adextension
  adplot
  adwidgets
  adportable
  adprocessor
  adcv
  portfolio
  qtwrapper
#  mpxcontrols
#  mpxprocessor
#  mpxwidgets
  ${QWT_LIBRARIES}
  ${Boost_LIBRARIES}
  ${OpenCV_LIBRARIES}
  Core
  extensionsystem
  utils
  Qt5::PrintSupport
  Qt5::Svg
  Qt5::Widgets
  Qt5::OpenGL
  Threads::Threads
  )

message( STATUS "###################################" )
message( STATUS "OPENCV_LIBRARIES: " ${OpenCV_LIBRARIES} )

set_target_properties( ${PROJECT_NAME}
  PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY_DEBUG   ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER} # for WIN32
  LIBRARY_OUTPUT_DIRECTORY_RELEASE ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER} # for WIN32
  LIBRARY_OUTPUT_DIRECTORY ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER} )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION lib/qtplatz/plugins/${PROVIDER} COMPONENT plugins
  LIBRARY DESTINATION lib/qtplatz/plugins/${PROVIDER} COMPONENT plugins
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )

install( FILES
  "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec"
  DESTINATION ${QTPLATZ_PLUGIN_DESTINATION}/${PROVIDER} COMPONENT plugins )
