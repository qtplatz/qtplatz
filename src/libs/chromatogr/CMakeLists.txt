# chromatogr
project( chromatogr )

cmake_policy(SET CMP0022 NEW)

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

runtime_install_path( _runtime_destination ${PROJECT_NAME} )
library_install_path( _library_destination ${PROJECT_NAME} )
archive_install_path( _archive_destination ${PROJECT_NAME} )

install( TARGETS ${PROJECT_NAME}
  EXPORT ${QTPLATZ_TARGETS}
  RUNTIME DESTINATION ${_runtime_destination} COMPONENT applications
  LIBRARY DESTINATION ${_library_destination} COMPONENT runtime_libraries
  ARCHIVE DESTINATION ${_archive_destination} COMPONENT libraries
  )
