#TARGET = qtwrapper
#TEMPLATE = lib
#CONFIG += staticlib
#include(../../qtplatzstaticlib.pri)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 CONFIG REQUIRED Core Widgets Gui Svg)

include_directories( ${QWT}/include
		     ${Qt5Widgets_INCLUDE_DIRS}
		   )


add_library( qtwrapper
  qtwrapper.cpp
  application.cpp
  qstring.cpp
  xmlformatter.cpp
  progressbar.cpp
  font.cpp
  settings.cpp
)

