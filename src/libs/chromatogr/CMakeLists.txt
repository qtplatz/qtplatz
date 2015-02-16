# chromatogr
project( chromatogr )

cmake_policy(SET CMP0022 NEW)

#find_package(Boost 1.57 REQUIRED COMPONENTS date_time system filesystem wserialization serialization)

add_definitions(-DCHROMATOGR_LIBRARY)

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
  simulator.cpp
  simulator.hpp
  )

target_link_libraries(
  chromatogr LINK_PUBLIC
  adcontrols
  adportable
  ${Boost_LIBRARIES} )

set_target_properties( chromatogr
  PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY ${QTPLATZ_BINARY_DIR}/lib/qtplatz )      

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT modules
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )
