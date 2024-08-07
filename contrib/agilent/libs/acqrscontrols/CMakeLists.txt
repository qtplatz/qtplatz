cmake_policy( SET CMP0028 NEW )
cmake_policy( SET CMP0020 NEW )

project( acqrscontrols )
set( PROVIDER MS-Cheminformatics )

add_definitions(-DACQRSCONTROLS_LIBRARY)
add_definitions( -D_LINUX -D_ACQIRIS )
if ( MSVC )
  add_definitions(-D_SCL_SECURE_NO_WARNINGS -wd4996 -wd4800 )
endif()

include_directories(
  ${CMAKE_SOURCE_DIR}/contrib/agilent/libs
  ${CMAKE_SOURCE_DIR}/src/libs
  ${AqDrv4_INCLUDE_DIR}
  )

# message( STATUS "### AqDrv4_INCLUDE_DIR: " ${AqDrv4_INCLUDE_DIR} )

add_library( ${PROJECT_NAME} SHARED
  acqiris_client.cpp
  acqiris_client.hpp
  acqiris_method.cpp
  acqiris_method.hpp
  acqiris_protocol.cpp
  acqiris_protocol.hpp
  acqiris_waveform.cpp
  acqiris_waveform.hpp
  acqrscontrols_global.hpp
  ap240/averagedata.cpp
  ap240/averagedata.hpp
  ap240/datainterpreter.cpp
  ap240/datainterpreter.hpp
  ap240/histogram.cpp
  ap240/histogram.hpp
  ap240/metadata.cpp
  ap240/metadata.hpp
  ap240/method.cpp
  ap240/method.hpp
  ap240/tdcdoc.cpp
  ap240/tdcdoc.hpp
  ap240/threshold_result.cpp
  ap240/threshold_result.hpp
  ap240/threshold_result_accessor.cpp
  ap240/threshold_result_accessor.hpp
  ap240/waveform.cpp
  ap240/waveform.hpp
  ap240/waveform_accessor.cpp
  ap240/waveform_accessor.hpp
  counting_data_writer.cpp
  counting_data_writer.hpp
  find_threshold_peaks.hpp
  find_threshold_timepoints.hpp
  pkd_counting_data_writer.cpp
  pkd_counting_data_writer.hpp
  processthreshold.cpp
  processthreshold.hpp
  resultwriter.hpp
  softaveraged_waveform_accessor.cpp
  softaveraged_waveform_accessor.hpp
  tdcbase.cpp
  tdcbase.hpp
  tdcdoc.cpp
  tdcdoc.hpp
  threshold_result.cpp
  threshold_result.hpp
  threshold_result_accessor.cpp
  threshold_result_accessor.hpp
  u5303a/averagedata.cpp
  u5303a/averagedata.hpp
  u5303a/device_method.cpp
  u5303a/device_method.hpp
  u5303a/histogram.cpp
  u5303a/histogram.hpp
  u5303a/identify.cpp
  u5303a/identify.hpp
  u5303a/metadata.cpp
  u5303a/metadata.hpp
  u5303a/method.cpp
  u5303a/method.hpp
  u5303a/tdcdoc.cpp
  u5303a/tdcdoc.hpp
  u5303a/threshold_result.cpp
  u5303a/threshold_result.hpp
  u5303a/waveform.cpp
  u5303a/waveform.hpp
  waveform_accessor.cpp
  waveform_accessor.hpp
  waveform_horizontal.hpp
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adacquire
  adlog
  adplugin
  adportable
  adcontrols
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

install(
  FILES
  acqiris_client.hpp
  acqiris_method.hpp
  acqiris_protocol.hpp
  acqiris_waveform.hpp
  acqrscontrols_global.hpp
  constants.hpp
  counting_data_writer.hpp
  datainterpreter.hpp
  find_threshold_peaks.hpp
  find_threshold_timepoints.hpp
  pkd_counting_data_writer.hpp
  processthreshold.hpp
  resultwriter.hpp
  softaveraged_waveform_accessor.hpp
  tdcbase.hpp
  tdcdoc.hpp
  threshold_action_finder.hpp
  threshold_peak_processor.hpp
  threshold_result_accessor.hpp
  threshold_result.hpp
  waveform_accessor.hpp
  waveform_horizontal.hpp
  DESTINATION include/acqrscontrols COMPONENT headers
  )

install(
  FILES
  ap240/averagedata.hpp
  ap240/datainterpreter.hpp
  ap240/histogram.hpp
  ap240/metadata.hpp
  ap240/method.hpp
  ap240/tdcdoc.hpp
  ap240/threshold_result.hpp
  ap240/threshold_result_accessor.hpp
  ap240/waveform.hpp
  ap240/waveform_accessor.hpp
  DESTINATION include/acqrscontrols/ap240 COMPONENT headers
  )

install(
  FILES
  u5303a/averagedata.hpp
  u5303a/device_method.hpp
  u5303a/histogram.hpp
  u5303a/identify.hpp
  u5303a/metadata.hpp
  u5303a/method.hpp
  u5303a/tdcdoc.hpp
  u5303a/threshold_result.hpp
  u5303a/waveform.hpp
  DESTINATION include/acqrscontrols/u5303a COMPONENT headers
  )
