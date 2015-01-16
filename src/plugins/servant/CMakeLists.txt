project( servant )
cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0028 NEW )

find_package( Qt5 CONFIG REQUIRED Core Network Script Sql )

set(CMAKE_AUTOMOC ON)

configure_file(
  "${PROJECT_SOURCE_DIR}/servant.pluginspec"
  "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/servant.pluginspec"
  )

execute_process( COMMAND xmlpatterns -no-format 
  -output ${PROJECT_SOURCE_DIR}/servant.json
  ${QTPLATZ_SOURCE_DIR}/src/pluginjsonmetadata.xsl
  ${PROJECT_SOURCE_DIR}/servant.pluginspec
  )

include_directories(
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${Qt5Core_INCLUDE_DIRS}
  )

if (WIN32)
  set( servant_LINK_DIRS ${QTPLATZ_BINARY_DIR}/lib/qtplatz )
elseif( ${CMAKE_SYSTEM_NAME} MATCHES Darwin )
  set( servant_LINK_DIRS ${QTPLATZ_SOURCE_DIR}/bin/qtplatz.app/Contents/Plugins/QtProject )
else()
  set( servant_LINK_DIRS ${QTPLATZ_BINARY_DIR}/lib/qtplatz/plugins/QtProject )
endif()

link_directories(
  ${servant_LINK_DIRS}
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
  Core
  ExtensionSystem
  Qt5::Core
)

#LIBS *= -lboost_serialization -lboost_date_time -lboost_filesystem -lboost_system
#OTHER_FILES += \
#    servant.pluginspec \
#    servant.config

