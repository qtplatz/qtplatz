#-------------------------------------------------
project( adsequence )
#
#  LIBS += -lboost_date_time -lboost_system -lboost_wserialization -lboost_serialization 
#
cmake_policy( SET CMP0022 NEW )

add_definitions( -DADSEQUENCE_LIBRARY )
include_directories( ${Qt5Core_INCLUDE_DIRS} )
find_package(Boost 1.57 REQUIRED COMPONENTS system wserialization serialization)

set( adsequence_SOURCES
  adsequence.cpp
  schema.cpp
  sequence.cpp
  streambuf.cpp
  )

set( adsequence_HEADERS
  adsequence.hpp
  adsequence_global.hpp
  schema.hpp
  sequence.hpp
  streambuf.hpp
  )

add_library( adsequence SHARED
  ${adsequence_SOURCES}
  ${adsequence_HEADERS}
  )

target_link_libraries( adsequence LINK_PUBLIC
  ${Boost_LIBRARIES} adportable
  )

#set_target_properties( adsequence
#  PROPERTIES
#  LIBRARY_OUTPUT_DIRECTORY ${QTPLATZ_BINARY_DIR}/lib/qtplatz )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT modules  
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )
