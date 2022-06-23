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

#find_package( Qt${QT_VERSION_MAJOR} CONFIG REQUIRED Core Widgets Svg Sql PrintSupport OpenGL )

add_definitions(-DADPUBLISHER_LIBRARY)

include_directories(
  ${QtConcurrent_INCLUDE_DIRS}
  ${QtSvg_INCLUDE_DIRS}
  ${QtPrintSupport_INCLUDE_DIRS}
  ${QtXml_INCLUDE_DIRS}
  ${QtXmlPatterns_INCLUDE_DIRS}
  ${QtCore_INCLUDE_DIRS}
  )

if ( NOT WIN32 )

  find_package( libxml2 )
  if ( NOT libxml2_FOUND )
    message( FATAL_ERROR "*** libxml2 NOT FOUND ***")
  endif()

  find_package( libxslt )
  if ( NOT libxslt_FOUND )
    message( FATAL_ERROR "*** libxslt NOT FOUND ***")
  endif()

  include_directories( ${libxml2_INCLUDE_DIRS} )
  include_directories( ${libxslt_INCLUDE_DIRS} )

  set( _xslt_sources libxslt_transformer.cpp libxslt_transformer.hpp )
  set( _xslt_libs ${libxml2_LIBRARIES} ${libxslt_LIBRARIES} )

else()

  set( _xslt_sources msxml_transformer.cpp msxml_transformer.hpp )

endif()

set( adpublisher_RESOURCES adpublisher.qrc )

add_library( adpublisher SHARED
  adpublisher_global.hpp
  adpublisher.hpp
  document.hpp
  document.cpp
  doceditor.hpp
  doceditor.cpp
  doctree.hpp
  doctree.cpp
  docedit.hpp
  docedit.cpp
  transformer.cpp
  transformer.hpp
  msxml_transformer.hpp
  printer.hpp
  printer.cpp
  textedit.cpp
  textedit.hpp
  ${_xslt_sources}
  ${adpublisher_RESOURCES}
  )

target_link_libraries(
  ${PROJECT_NAME} LINK_PUBLIC
  adportable
  qtwrapper
  xmlparser
  Qt::Widgets
  Qt::Core
  Qt::PrintSupport
  ${_xslt_libs}
  ${Boost_LIBRARIES}
  )

if ( NOT WIN32 )
  add_executable( adedit
    main.cpp
    )

target_link_libraries(
  adedit LINK_PUBLIC
  ${PROJECT_NAME}
  )
endif()


install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT modules
  ARCHIVE DESTINATION lib COMPONENT libraries )
