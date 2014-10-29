#TARGET = qtwrapper
#TEMPLATE = lib
#CONFIG += staticlib
#include(../../qtplatzstaticlib.pri)

add_library( qtwrapper
  qtwrapper.cpp
  application.cpp
  qstring.cpp
  xmlformatter.cpp
  progressbar.cpp
  font.cpp
  settings.cpp
)

