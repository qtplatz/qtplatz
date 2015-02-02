# chromatogr
project( chromatogr )

cmake_policy(SET CMP0022 NEW)

find_package( Qt5 REQUIRED Core )
#find_package(Boost 1.57 REQUIRED COMPONENTS date_time system filesystem wserialization serialization)

add_definitions(-DCHROMATOGR_LIBRARY)

include_directories( ${Qt5Core_INCLUDE_DIRS} )

add_library( chromatogr SHARED 
  chromatogr.cpp
  chromatography.cpp
  integrator.cpp
  chromatogr.hpp
  chromatogr_global.hpp
  chromatography.hpp
  differential.hpp
  averager.hpp
  integrator.hpp  
  )

target_link_libraries(
  chromatogr LINK_PUBLIC
  adcontrols
  adportable
  ${Boost_LIBRARIES} )


install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )
