
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
  http_client.cpp
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
  threshold_finder.hpp
  timer.cpp
  timesquaredscanlaw.cpp
  utf.cpp
  uuid.cpp
  fft4g_h.c
  waveform_averager.hpp
  waveform_processor.hpp
  waveform_simulator.hpp
  waveform_simulator.cpp
  waveform_wrapper.hpp
  )

target_include_directories( adportable PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )
