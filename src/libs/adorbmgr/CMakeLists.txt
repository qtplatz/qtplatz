
cmake_policy(SET CMP0022 NEW)
add_definitions(-DADORBMGR_LIBRARY)

find_package( ace+tao )

if ( ace+tao_FOUND )
  include_directories( ${ACE+TAO_INCLUDE_DIRS} )
  link_directories( ${ACE+TAO_LIBRARY_DIRS} )
endif()

add_library( adorbmgr SHARED
  adorbmgr_global.h
  adorbmgr.cpp
  adorbmgr.h
  orbmgr.cpp
  orbmgr.hpp
  )

target_link_libraries( adorbmgr LINK_PUBLIC
  acewrapper
  adlog
  adportable
  xmlparser
  TAO_Utils
  TAO_PortableServer
  TAO_AnyTypeCode
  TAO
  ACE
  ${Boost_LIBRARIES} )

#LIBS += -l$$qtLibraryTarget(acewrapper) \
#        -l$$qtLibraryTarget(adlog) \
#        -l$$qtLibraryTarget(adportable) \
#        -l$$qtLibraryTarget(qtwrapper) \
#        -l$$qtLibraryTarget(xmlparser)

#!win32: LIBS += -lboost_thread -lboost_system
