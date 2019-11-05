cmake_policy( SET CMP0028 NEW )
cmake_policy( SET CMP0020 NEW )

project( aqmd3controls )
set( PROVIDER MS-Cheminformatics )

add_definitions(-DAQMD3CONTROLS_LIBRARY)
add_definitions( -D_LINUX -D_ACQIRIS )
if ( MSVC )
  add_definitions(-D_SCL_SECURE_NO_WARNINGS -wd4996 -wd4800 )
endif()

include_directories(
  ${CMAKE_SOURCE_DIR}/contrib/acqiris/libs
  ${CMAKE_SOURCE_DIR}/contrib/agilent/libs
  ${CMAKE_SOURCE_DIR}/src/libs
  ${AqDrv4_INCLUDE_DIR}
  )

add_library( ${PROJECT_NAME} SHARED
  counting_data_writer.cpp
  counting_data_writer.hpp
  pkd_result.cpp
  pkd_result.hpp
  pkd_result_accessor.cpp
  pkd_result_accessor.hpp
#  find_threshold_peaks.hpp
#  find_threshold_timepoints.hpp
#  pkd_counting_data_writer.cpp
#  pkd_counting_data_writer.hpp
#  processthreshold.cpp
#  processthreshold.hpp
#  resultwriter.hpp
#  softaveraged_waveform_accessor.cpp
#  softaveraged_waveform_accessor.hpp
#  tdcbase.cpp
#  tdcbase.hpp
#  tdcdoc.cpp
#  tdcdoc.hpp
  threshold_result.cpp
  threshold_result.hpp
#  threshold_result_accessor.cpp
#  threshold_result_accessor.hpp

#  averagedata.cpp
#  averagedata.hpp
  device_method.cpp
  device_method.hpp
  histogram.cpp
  histogram.hpp
  identify.cpp
  identify.hpp
  meta_data.cpp
  meta_data.hpp
  method.cpp
  method.hpp
#  tdcdoc.cpp
#  tdcdoc.hpp
#  threshold_result.cpp
#  threshold_result.hpp
  waveform.cpp
  waveform.hpp
#  waveform_accessor.cpp
#  waveform_accessor.hpp
#  waveform_horizontal.hpp
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adacquire
  adplugin
  adportable
  adcontrols
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT runtime_libraries
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )
