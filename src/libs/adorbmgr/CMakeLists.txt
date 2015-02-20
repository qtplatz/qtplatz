project( adorbmgr )
cmake_policy(SET CMP0022 NEW)
add_definitions(-DADORBMGR_LIBRARY)

find_package( ace+tao )

if ( ace+tao_FOUND )
  include_directories( ${ACE+TAO_INCLUDE_DIRS} )
  link_directories( ${ACE+TAO_LIBRARY_DIRS} )
else()
  message( FATAL_ERROR "ACE+TAO Not found" )
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
  ${ACE+TAO_LIBRARIES}
  ${Boost_LIBRARIES} )

#set_target_properties( ${PROJECT_NAME}
#  PROPERTIES
#  LIBRARY_OUTPUT_DIRECTORY ${QTPLATZ_BINARY_DIR}/lib/qtplatz )      

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT modules
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )

#LIBS += -l$$qtLibraryTarget(acewrapper) \
#        -l$$qtLibraryTarget(adlog) \
#        -l$$qtLibraryTarget(adportable) \
#        -l$$qtLibraryTarget(qtwrapper) \
#        -l$$qtLibraryTarget(xmlparser)

#!win32: LIBS += -lboost_thread -lboost_system
