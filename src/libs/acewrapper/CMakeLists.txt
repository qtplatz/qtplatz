#TARGET = acewrapper
#TEMPLATE = lib
#CONFIG += staticlib
#win32: QMAKE_CXXFLAGS += -D_CRT_SECURE_NO_WARNINGS
#INCLUDEPATH *= $$OUT_PWD/..

add_library( acewrappers 
  input_buffer.cpp
  ifconfig.cpp
  ifconfig_windows.cpp
  iorsender.cpp
  iorquery.cpp
)
           
