
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
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )

install( FILES
  debug.hpp
  float.hpp
  fft.hpp
  formula_parser.hpp
  graycode.hpp
  base64.hpp
  base85.hpp
  binary_search.hpp
  DESTINATION include/adportable COMPONENT headers
  )

#adconv.hpp             configuration.hpp        http_client.hpp               portable_binary_oarchive.hpp  serialport.hpp          uuid.hpp
#advance.hpp            constants.hpp            is_equal.hpp                  posix_path.hpp                sgfilter.hpp            waveform_averager.hpp
#array_wrapper.hpp      date_string.hpp          is_type.hpp                   profile.hpp                   singleton.hpp           waveform_peakfinder.hpp
#average.hpp            debug_core.hpp           lifecycleframe.hpp            protocollifecycle.hpp         spectrum_processor.hpp  waveform_processor.hpp
#base64.hpp             debug.hpp                mblock.hpp                    safearray.hpp                 split_filename.hpp      waveform_simulator.hpp
#base85.hpp             fft.hpp                  moment.hpp                    scoped_debug.hpp              string.hpp              waveform_wrapper.hpp
#binary_search.hpp      float.hpp                mslockref.hpp                 scoped_flag.hpp               textfile.hpp            xml_serializer.hpp
#binary_serializer.hpp  formula_parser.hpp       pkfinder.hpp                  scope_timer.hpp               timer.hpp
#bzip2.hpp              gdiostate.hpp            polfit.hpp                    semaphore.hpp                 timesquaredscanlaw.hpp
#combination.hpp        graycode.hpp             portable_binary_archive.hpp   serialization_shared_ptr.hpp  unique_ptr.hpp
#configloader.hpp       histogram_processor.hpp  portable_binary_iarchive.hpp  serializer.hpp                utf.hpp
