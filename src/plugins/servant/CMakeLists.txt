cmake_policy( SET CMP0020 NEW )
project( servant )

find_package( Qt5 CONFIG REQUIRED Core )

set(CMAKE_AUTOMOC ON)

configure_file(
  "${PROJECT_SOURCE_DIR}/servant.pluginspec"
  "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/servant.pluginspec"
  )

include_directories(
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${Qt5Core_INCLUDE_DIRS}
  )

link_directories(
  ${QTPLATZ_BINARY_DIR}/lib/qtplatz/plugins/QtProject
  ${QTPLATZ_BINARY_DIR}/lib/qtplatz  # <== qmake .lib output dir
  )

add_definitions( -DSERVANT_LIBRARY )

add_library( servant MODULE
  outputwindow.cpp
  servant.cpp
  servantmode.cpp
  servantplugin.cpp
  logger.cpp
  servant_global.h
  outputwindow.hpp
  servant.hpp
  servantmode.hpp
  servantplugin.hpp
  logger.hpp
  )

target_link_libraries( servant LINK_PUBLIC
  ${Boost_LIBRARIES}
  adcontrols
  adextension
  adportable
  adplugin
  adlog
  qtwrapper
  adextension
  xmlparser
  optimized Core debug Cored
  ExtensionSystem
  Qt5::Core
)

#LIBS *= -lboost_serialization -lboost_date_time -lboost_filesystem -lboost_system
#OTHER_FILES += \
#    servant.pluginspec \
#    servant.config

