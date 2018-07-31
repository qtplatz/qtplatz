
project( adportable )

add_library( adportable
  array_wrapper.hpp
  base64.cpp
  bzip2.cpp
  configloader.cpp
  configuration.cpp
  constants.cpp
  ConvertUTF.c
  date_string.cpp
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
  portable_binary_oarchive.cpp
  portable_binary_oarchive.hpp
  portable_binary_iarchive.cpp
  portable_binary_iarchive.hpp
  posix_path.cpp
  profile.cpp
  semaphore.hpp
  serialport.cpp
  sgfilter.cpp
  spectrum_processor.cpp
  string.cpp
  textfile.cpp
  counting/threshold_finder.hpp
  counting/threshold_index.hpp
  timer.cpp
  timesquaredscanlaw.cpp
  utf.cpp
  uuid.cpp
  fft4g_h.c
  waveform_averager.hpp
  waveform_peakfinder.cpp  
  waveform_peakfinder.hpp
  waveform_processor.hpp
  waveform_simulator.hpp
  waveform_simulator.cpp
  waveform_wrapper.hpp
  )

target_include_directories( adportable PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  ARCHIVE DESTINATION lib COMPONENT libraries ) # .a

install( FILES
  debug.hpp
  float.hpp
  fft.hpp
  formula_parser.hpp
  graycode.hpp
  base64.hpp
  base85.hpp
  binary_search.hpp
  portable_binary_archive.hpp
  portable_binary_iarchive.hpp
  portable_binary_oarchive.hpp
  binary_serializer.hpp
  serializer.hpp
  DESTINATION include/adportable COMPONENT headers
  )

