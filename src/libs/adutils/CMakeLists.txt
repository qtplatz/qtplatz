# adutils
project ( adutils )

add_definitions( -DADUTILS_LIBRARY )

add_library( ${PROJECT_NAME} SHARED
  adutils.cpp
  adutils.hpp
  acquiredconf.cpp
  acquiredconf.hpp
  acquiredconf_v3.cpp
  acquiredconf_v3.hpp
  acquireddata.cpp
  acquireddata.hpp
  acquireddata_v3.hpp
  acquireddata_v3.cpp
  adfile.cpp
  adfile.hpp
  processeddata.cpp
  cpio.cpp
  cpio.hpp
  fsio.cpp
  fsio.hpp
  fsio2.cpp
  fsio2.hpp
  inifile.cpp
  inifile.hpp
  mscalibio.cpp
  mscalibio.hpp
  processeddata.hpp
  processeddata_t.hpp
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adportable
  adfs
  adcontrols
  portfolio
  ${Boost_LIBRARIES}
  Threads::Threads
  )

runtime_install_path( _runtime_destination ${PROJECT_NAME} )
library_install_path( _library_destination ${PROJECT_NAME} )
archive_install_path( _archive_destination ${PROJECT_NAME} )

install( TARGETS ${PROJECT_NAME}
  EXPORT ${QTPLATZ_TARGETS}
  RUNTIME DESTINATION ${_runtime_destination} COMPONENT applications
  LIBRARY DESTINATION ${_library_destination} COMPONENT runtime_libraries
  ARCHIVE DESTINATION ${_archive_destination} COMPONENT libraries
  )
