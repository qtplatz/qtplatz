project( acewrapper )
#TARGET = acewrapper
#TEMPLATE = lib
#CONFIG += staticlib
#win32: QMAKE_CXXFLAGS += -D_CRT_SECURE_NO_WARNINGS
#INCLUDEPATH *= $$OUT_PWD/..

add_library( acewrapper
  constants.cpp
  input_buffer.cpp
  ifconfig.cpp
  ifconfig_windows.cpp
  iorsender.cpp
  iorquery.cpp
  udpeventreceiver.cpp
  udpeventreceiver.hpp
  udpeventsender.cpp
  udpeventsender.hpp
)
           
install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT apprications
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )
