# adutils
project ( adutils )

add_library( adutils 
  adutils.cpp
  adutils.hpp
  acquiredconf.cpp
  acquiredconf.hpp
  acquireddata.cpp
  acquireddata.hpp
  adfile.cpp
  adfile.hpp
  processeddata.cpp
  cpio.cpp
  cpio.hpp
  fsio.cpp
  fsio.hpp
  fsio2.cpp
  fsio2.hpp
  mscalibio.cpp
  mscalibio.hpp
  processeddata.hpp
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )
