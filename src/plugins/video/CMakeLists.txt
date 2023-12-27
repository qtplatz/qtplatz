cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0028 NEW )
project( video )
set( PROVIDER MS-Cheminformatics )
set( COMPAT_VERSION 3.2.81 )
set( VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )

find_package( Qt${QT_VERSION_MAJOR} CONFIG REQUIRED Core Svg Sql PrintSupport Widgets OpenGL Multimedia )

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB UIs *.ui)
file(GLOB Resources *.qrc)

if ( OpenCV_FOUND )
  include_directories( ${OpenCV_INCLUDE_DIRS} )
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

qtplatz_plugin_output_dir( _plugin_output_dir ${PROVIDER} )
qtplatz_plugin_install_dir( _plugin_install_dir ${PROVIDER} )

########## QtCreator 8 (Qt5/6) plugin.json #############
if ( IDE_VERSION VERSION_GREATER_EQUAL "8.0.0" )
  list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/cmake" )
  find_package( QtCreator REQUIRED COMPONENTS Core )
  configure_file(
    "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.json.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.json"
  )
else()
  message( FATAL_ERROR "IDE_VERSION" )
endif()
####################### END ##########################

#find_package( qwt )
add_definitions(  -DVIDEO_LIBRARY  )

if ( WIN32 )
  add_definitions( -D_SCL_SECURE_NO_WARNINGS )
endif()

include_directories(
  ${QWT_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${QtOpenGL_INCLUDE_DIRS}
  ${QtCore_INCLUDE_DIRS}
  ${QtWidgets_INCLUDE_DIRS}
  ${QtPrintSupport_INCLUDE_DIRS}
  ${Qt${QT_VERSION_MAJOR}Multimedia_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src         # version.h
  ${CMAKE_BINARY_DIR}/src/libs
  ${CMAKE_SOURCE_DIR}/src/libs
  ${CMAKE_SOURCE_DIR}/src/adplugins
  ${QTPLATZ_SOURCE_DIR}/src/plugins
  ${QTPLATZ_SOURCE_DIR}/src      # version number
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
  imageview.cpp
  imageview.hpp
  mainwindow.cpp
  mainwindow.hpp
  mode.cpp
  mode.hpp
  player.cpp
  player.hpp
  processor.cpp
  processor.hpp
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
  adwidgets
  adportable
  adprocessor
  adcv
  portfolio
  qtwrapper
  ${Boost_LIBRARIES}
  ${OpenCV_LIBRARIES}
  Core
  extensionsystem
  utils
  Qt::PrintSupport
  Qt::Svg
  Qt::Widgets
  Qt::OpenGL
  Qt::Multimedia
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
