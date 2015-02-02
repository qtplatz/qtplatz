# adutils
project ( adutils )

add_library( adutils 
  adutils.cpp
  processeddata.cpp
  adfile.cpp
  cpio.cpp
  fsio.cpp
  fsio2.cpp
  mscalibio.cpp
  acquiredconf.cpp
  acquireddata.cpp
  adutils.hpp
  processeddata.hpp
  adfile.hpp
  cpio.hpp
  fsio.hpp
  fsio2.hpp
  mscalibio.hpp
  acquiredconf.hpp
  acquireddata.hpp
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT apprications
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )
