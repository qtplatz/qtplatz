
add_library( adportable
  base64.cpp
  configloader.cpp
  configuration.cpp
  constants.cpp
  ConvertUTF.c
  date_string.cpp
  debug.cpp
  fft.cpp
  mslockref.cpp  
  mslockref.hpp
  polfit.cpp
  portable_binary_oarchive.cpp
  portable_binary_iarchive.cpp
  posix_path.cpp
  serialport.cpp
  spectrum_processor.cpp
  string.cpp
  timesquaredscanlaw.cpp
  utf.cpp
  lifecycleframe.cpp
  timer.cpp
  profile.cpp
  textfile.cpp
  bzip2.cpp
  debug_core.cpp
  sgfilter.cpp
  uuid.cpp )

target_include_directories( adportable PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )
