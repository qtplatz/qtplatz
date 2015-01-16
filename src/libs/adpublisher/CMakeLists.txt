# adpublisher
project( adpublisher )

cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0028 NEW)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

configure_file(
  "${PROJECT_SOURCE_DIR}/xslt/debug.xslt"
  "${PROJECT_BINARY_DIR}/share/qtplatz/xslt"  # <QtPlatz>/share/qtplatz/xslt
  )

find_package( Qt5 CONFIG REQUIRED Concurrent Sql Svg Xml PrintSupport XmlPatterns WebKitWidgets WebKit Core )
find_package( Boost 1.57 REQUIRED COMPONENTS date_time system filesystem )

add_definitions(-DADPUBLISHER_LIBRARY)

include_directories(
  ${Qt5Concurrent_INCLUDE_DIRS}
  ${Qt5Svg_INCLUDE_DIRS}
  ${Qt5PrintSupport_INCLUDE_DIRS}
  ${Qt5Xml_INCLUDE_DIRS}
  ${Qt5XmlPatterns_INCLUDE_DIRS}
  ${Qt5WebKitWidgets_INCLUDE_DIRS}
  ${Qt5Core_INCLUDE_DIRS}
  )

set( adpublisher_SOURCES
  adpublisher.cpp
  document.cpp
  docbrowser.cpp
  doceditor.cpp
  doctree.cpp
  docedit.cpp
  transformer.cpp
  )

set( adpublisher_HEADERS
  adpublisher.hpp
  adpublisher_global.hpp 
  document.hpp 
  docbrowser.hpp 
  doceditor.hpp 
  doctree.hpp 
  docedit.hpp 
  transformer.hpp 
  msxml_transformer.hpp
  )

if (${CMAKE_SYSTEM_NAME} MATCHES Darwin )
  include_directories( /opt/local/include/libxml2 )
  link_directories( /opt/local/lib )
endif()

set( adpublisher_RESOURCES adpublisher.qrc )

if( WIN32 )
  set( adpublisher_SOURCES ${adpublisher_SOURCES} msxml_transformer.cpp )
  set( adpublisher_HEADERS ${adpublisher_HEADERS} msxml_transformer.hpp )
else()
  set( adpublisher_SOURCES ${adpublisher_SOURCES} libxslt_transformer.cpp )
  set( adpublisher_HEADERS ${adpublisher_HEADERS} libxslt_transformer.hpp )
endif()

add_library( adpublisher SHARED
  ${adpublisher_SOURCES}
  ${adpublisher_HEADERS}
  ${adpublisher_RESOURCES}  
  )

if (NOT WIN32)
  set(adpublisher_LIBS xslt xml2)
endif()

target_link_libraries(
  adpublisher LINK_PUBLIC
  adcontrols
  adportable
  xmlparser
  Qt5::Widgets
  Qt5::Core
  Qt5::WebKit
  Qt5::WebKitWidgets
  Qt5::PrintSupport
  ${Boost_LIBRARIES}
  ${adpublisher_LIBS}
)

#macx {
# check with command: xml2-config --cflags --libs
#  LIBS += -L/opt/local/lib -lxml2 -lz -lpthread -liconv -lm
#  INCLUDEPATH += /opt/local/include/libxml2
#}

#linux-* {
##  message("QMAKESPEC=" $$QMAKESPEC)
##  message("QMAKE_CC=" $$QMAKE_CC)
## check with command: xml2-config --cflags --libs
## for cross compile, --sysroot will be automatically added
#  INCLUDEPATH += /usr/include/libxml2
#}
