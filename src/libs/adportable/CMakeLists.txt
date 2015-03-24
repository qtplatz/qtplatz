
project( adportable )

add_library( adportable
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
  mslockref.cpp  
  mslockref.hpp
  polfit.cpp
  portable_binary_oarchive.cpp
  portable_binary_iarchive.cpp
  posix_path.cpp
  profile.cpp
  serialport.cpp
  sgfilter.cpp
  spectrum_processor.cpp
  string.cpp
  textfile.cpp
  timer.cpp
  timesquaredscanlaw.cpp
  utf.cpp
  uuid.cpp
  )

target_include_directories( adportable PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )
