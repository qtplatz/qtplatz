project( adcontroller )

add_definitions(-DADCONTROLLER_LIBRARY)

if ( MSVC )
  add_definitions( -D_SCL_SECURE_NO_WARNINGS )
endif()

find_package( ace+tao )
find_package( Qt5 REQUIRED Core )
include_directories(
  ${ACE+TAO_INCLUDE_DIRS}
  ${Qt5Core_INCLUDE_DIRS}
  )


add_library( adcontroller MODULE
  adcontroller.cpp
  cache.cpp
  fileio.cpp
  iproxy.cpp
  manager_i.cpp
  observer_i.cpp
  oproxy.cpp
  receiver_i.cpp
  sampleprocessor.cpp
  session_i.cpp
  task.cpp
  logging.cpp
  adcontroller.hpp
  adcontroller_global.h
  cache.hpp
  fileio.hpp
  iproxy.hpp
  manager_i.hpp
  observer_i.hpp
  oproxy.hpp
  receiver_i.hpp
  sampleprocessor.hpp
  session_i.hpp
  task.hpp
  logging.hpp
  )

if ( CMAKE_SYSTEM_NAME EQUAL Linux )
  set( linux_LIBS dl rt )
endif()

link_directories( ${ACE+TAO_LIBRARY_DIRS} )

target_link_libraries( adcontroller LINK_PUBLIC
  adinterface
  adfs
  adcontrols
  adportable
  adutils
  acewrapper
  adplugin
  xmlparser
  adlog
  optimized TAO_Utils debug TAO_Utilsd
  optimized TAO_PI debug TAO_PId
  optimized TAO_PortableServer debug TAO_PortableServerd
  optimized TAO_AnyTypeCode debug TAO_AnyTypeCoded
  optimized TAO debug TAOd
  optimized ACE debug ACEd
  ${Boost_LIBRARIES}
  ${linux_LIBS}
)


