cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0028 NEW )

project( video )
set( PROVIDER MS-Cheminformatics )
set( COMPAT_VERSION 3.2.81 )
set( VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )

find_package( Qt5 CONFIG REQUIRED Core Svg Sql PrintSupport Widgets OpenGL Multimedia )

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 14)

file(GLOB UIs *.ui)
file(GLOB Resources *.qrc)

if ( OpenCV_FOUND )
  find_package( OpenCV 3 REQUIRED core imgproc video highgui features2d )
  add_definitions( "-DOPENCV=1" )
  include_directories( ${OpenCV_INCLUDE_DIRS} ${HDF5_INCLUDE_DIRS} )
endif()

if ( CUDA_FOUND )
  list( APPEND CUDA_NVCC_FLAGS "-std=c++11;-O2;-DVERBOSE")
  set ( CUDA_PROPAGATE_HOST_FLAGS OFF )
  include_directories (
    ${CUDA_TOOLKIT_ROOT_DIR}/samples/common/inc
    ${CUDA_INCLUDE_DIRS}
  )
endif()

if ( ArrayFire_FOUND )
  include_directories ( ${ArrayFire_INCLUDE_DIRS} )
endif()

configure_file(
  "${PROJECT_SOURCE_DIR}/../pluginspec.in"
  "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec"
  )

execute_process( COMMAND ${XMLPATTERNS} -no-format 
  -output ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.json
  ${QTPLATZ_SOURCE_DIR}/src/pluginjsonmetadata.xsl
  ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec  
  )

find_package( qwt )

add_definitions(  -DVIDEO_LIBRARY  )

if ( WIN32 )
  add_definitions( -D_SCL_SECURE_NO_WARNINGS )
endif()

include_directories(
  ${QWT_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${Qt5OpenGL_INCLUDE_DIRS}
  ${Qt5Core_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}
  ${Qt5PrintSupport_INCLUDE_DIRS}
  ${Qt5Multimedia_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src         # version.h
  ${CMAKE_BINARY_DIR}/src/libs
  ${CMAKE_SOURCE_DIR}/src/libs
  ${CMAKE_SOURCE_DIR}/src/adplugins
  ${QTPLATZ_SOURCE_DIR}/src/plugins
  ${QTPLATZ_SOURCE_DIR}/src      # version number
  )

link_directories(
  ${QTPLATZ_BINARY_DIR}/lib/qtplatz/plubins/QtProject
  ${QTPLATZ_BINARY_DIR}/lib/qtplatz  # <== qmake .lib output dir
  )

set( RESOURCES video.qrc )

add_library( ${PROJECT_NAME} MODULE
  constants.hpp
  cvmat.cpp
  cvmat.hpp
  document.cpp
  document.hpp
  dft2d.cpp
  dft2d.hpp
  imagewidget.cpp
  imagewidget.hpp
  imageview.cpp
  imageview.hpp
  mainwindow.cpp
  mainwindow.hpp
  mode.cpp
  mode.hpp
  player.cpp
  player.hpp
  playercontrols.cpp
  playercontrols.hpp
  recorder.cpp
  recorder.hpp
  recordercontrols.cpp
  recordercontrols.hpp
  videocapturewnd.hpp
  videocapturewnd.cpp
  videoeditor.cpp
  videoeditor.hpp
  videofactory.cpp
  videofactory.hpp
  videoplugin.cpp
  videoplugin.hpp
  videoprocwnd.hpp
  videoprocwnd.cpp
  ${RESOURCES}
  )

target_link_libraries(
  ${PROJECT_NAME} LINK_PUBLIC
  ${libcuda}
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
  advision
  portfolio
  qtwrapper
  ${Boost_LIBRARIES}
  ${OpenCV_LIBRARIES}
  ${HDF5_LIBRARIES}
  Core
  extensionsystem
  utils
  Qt5::PrintSupport
  Qt5::Svg
  Qt5::Widgets
  Qt5::OpenGL
  Qt5::Multimedia
  )
  
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
