#
# CMakeLists for adplugin shared library
#
project( adacquire )

cmake_policy(SET CMP0028 NEW)
cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0020 NEW)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_definitions(-DADICONTROLLER_LIBRARY)
if (WIN32)
  add_definitions( -D_SCL_SECURE_NO_WARNINGS )
endif()

add_library( ${PROJECT_NAME} SHARED
  adacquire_global.hpp
  adacquirefactory.cpp
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
  time_event_processor.hpp
  time_event_processor.cpp
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
  adportable
  ${Boost_LIBRARIES}
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

install( FILES
  constants.hpp
  DESTINATION include/adacquire COMPONENT headers
  )
