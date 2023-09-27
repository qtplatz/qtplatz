
project( adportable )

add_definitions(-DADPORTABLE_LIBRARY)

if ( MSVC )
  set( CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE ) # <-- quick workaround for undefined symbols to extract( .., boosst::uuids::uuid, ...)
endif()

add_library( ${PROJECT_NAME} SHARED
  "${CMAKE_CURRENT_SOURCE_DIR}/../compiler/decl_export.h"
  adportable_global.h
  array_wrapper.hpp
  base64.cpp
  bzip2.cpp
  configloader.cpp
  configuration.cpp
  constants.cpp
  ConvertUTF.c
  date_string.cpp
  date_time.cpp
  debug.cpp
  debug_core.cpp
  fft.cpp
  graycode.cpp
  graycode.hpp
  http_client.cpp
  histogram_processor.cpp
  histogram_processor.hpp
  lifecycleframe.cpp
  mblock.hpp
  mslockref.cpp
  mslockref.hpp
  polfit.cpp
  profile.cpp
  semaphore.hpp
  serialport.cpp
  sgfilter.cpp
  spectrum_processor.cpp
  string.cpp
  textfile.cpp
  counting/threshold_finder.hpp
  counting/threshold_index.hpp
  csv_reader.cpp  csv_reader.hpp
  timer.cpp
  timesquaredscanlaw.cpp
  utf.cpp
  uuid.cpp
  waveform_averager.hpp
  waveform_peakfinder.cpp
  waveform_peakfinder.hpp
  waveform_processor.hpp
  waveform_simulator.hpp
  waveform_simulator.cpp
  waveform_wrapper.hpp
  json_helper.cpp
  json_helper.hpp
  json/extract.cpp
  json/extract.hpp
  tuple_arith.hpp
  fft4g.cpp fft4g.hpp
  fft/fft4g.c
  # fft4g_h.c
)

if ( MSVC )
  target_compile_definitions( ${PROJECT_NAME} INTERFACE -DBOOST_JSON_DYN_LINK )
endif()

add_library( QtPlatz::${PROJECT_NAME} ALIAS ${PROJECT_NAME} )

target_include_directories( adportable PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  xmlparser
  ${Boost_LIBRARIES}
  Threads::Threads
  )

# message( STATUS "Boost_LIBRARIES = " ${Boost_LIBRARIES} )
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
  adportable_global.h
  base64.hpp
  base85.hpp
  binary_search.hpp
  binary_serializer.hpp
  date_time.hpp
  debug.hpp
  fft.hpp
  float.hpp
  formula_parser.hpp
  graycode.hpp
  iso8601.hpp
  serializer.hpp
  DESTINATION include/adportable COMPONENT headers
  )

#[[
adconv.hpp
advance.hpp
array_wrapper.hpp
average.hpp
base64.hpp
base85.hpp
basic_waveform.hpp
binary_search.hpp
binary_serializer.hpp
bzip2.hpp
combination.hpp
configloader.hpp
configuration.hpp
constants.hpp
countrate_calculator.hpp
date_string.hpp
date_time.hpp
debug.hpp
debug_core.hpp
fft.hpp
find_threshold_peaks.hpp
find_threshold_timepoints.hpp
float.hpp
for_each_combination.hpp
formula_parser.hpp
gdiostate.hpp
graycode.hpp
histogram_processor.hpp
http_client.hpp
is_equal.hpp
is_same.hpp
is_type.hpp
lifecycleframe.hpp
mass_assign_t.hpp
mblock.hpp
moment.hpp
mslockref.hpp
optional.hpp
pkfinder.hpp
polfit.hpp
profile.hpp
protocollifecycle.hpp
safearray.hpp
scanlaw_solver.hpp
scope_timer.hpp
scoped_debug.hpp
scoped_flag.hpp
semaphore.hpp
serialization_shared_ptr.hpp
serializer.hpp
serialport.hpp
sgfilter.hpp
singleton.hpp
spectrum_processor.hpp
split_filename.hpp
string.hpp
textfile.hpp
time_format.hpp
timer.hpp
timesquaredscanlaw.hpp
unique_ptr.hpp
utf.hpp
uuid.hpp
waveform_averager.hpp
waveform_peakfinder.hpp
waveform_processor.hpp
waveform_simulator.hpp
waveform_wrapper.hpp
xml_serializer.hpp
]]
