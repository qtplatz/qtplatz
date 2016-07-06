#
# CMakeLists for adplugin shared library
#
project( adicontroller )

cmake_policy(SET CMP0028 NEW)
cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0020 NEW)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Boost 1.57 REQUIRED COMPONENTS wserialization serialization bzip2 iostreams )

add_definitions(-DADICONTROLLER_LIBRARY)
if (WIN32)
  add_definitions( -D_SCL_SECURE_NO_WARNINGS )
endif()

add_library( ${PROJECT_NAME} SHARED
  adicontroller.cpp
  adicontroller_global.hpp
  automaton.hpp
  datawriter.hpp
  datawriter.cpp
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
  samplesequence.hpp
  samplesequence.cpp
  signalobserver.hpp
  signalobserver.cpp
  simpleobserver.hpp
  simpleobserver.cpp
  task.hpp
  task.cpp
  timedigital_histogram_accessor.hpp
  timedigital_histogram_accessor.cpp
  waveform_simulator.hpp
  waveform_simulator.cpp
  waveform_simulator_manager.hpp
  waveform_simulator_manager.cpp
  writeaccess.hpp
  writeaccess.cpp
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  acewrapper
  adplugin
  adfs
  adcontrols
  adutils
  ${Boost_LIBRARIES} )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT modules
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )

