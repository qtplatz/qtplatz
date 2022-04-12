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

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT runtime_libraries
  ARCHIVE DESTINATION lib COMPONENT libraries )
