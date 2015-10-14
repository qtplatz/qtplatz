#
# CMakeLists for adplugin shared library
#
project( adicontroller )

cmake_policy(SET CMP0028 NEW)
cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0020 NEW)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Boost 1.57 REQUIRED COMPONENTS wserialization serialization)

add_definitions(-DADICONTROLLER_LIBRARY)
if (WIN32)
  add_definitions( -D_SCL_SECURE_NO_WARNINGS )
endif()

add_library( ${PROJECT_NAME} SHARED
  adicontroller.cpp
  adicontroller_global.hpp
  acquiredconf_v3.hpp
  acquiredconf_v3.cpp
  acquireddata_v3.hpp
  acquireddata_v3.cpp
  eventlog.hpp
  eventlog.cpp
  instrument.hpp
  instrument.cpp
  manager.hpp
  manager.cpp
  masterobserver.hpp
  masterobserver.cpp
  mscalibio_v3.hpp
  mscalibio_v3.cpp
  octet_array.hpp
  octet_array.cpp
  receiver.hpp
  receiver.cpp
  sampleprocessor.hpp
  sampleprocessor.cpp
  signalobserver.hpp
  signalobserver.cpp
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC adplugin adfs adcontrols adutils ${Boost_LIBRARIES} )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT modules
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )

