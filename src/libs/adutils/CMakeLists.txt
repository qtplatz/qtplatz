# adutils
project ( adutils )

add_library( adutils
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
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  ARCHIVE DESTINATION lib COMPONENT libraries
  )
