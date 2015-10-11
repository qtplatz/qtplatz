cmake_policy( SET CMP0028 NEW )
cmake_policy( SET CMP0022 NEW )

project( adcontrols )


find_package(Boost 1.57 REQUIRED COMPONENTS date_time system filesystem wserialization serialization)

add_definitions(-DADCONTROLS_LIBRARY)
if ( MSVC )
  add_definitions(-D_SCL_SECURE_NO_WARNINGS)  
endif()

add_library( adcontrols SHARED
  acceptor.cpp 
  acquireddataset.cpp 
  adcontrols.cpp 
  baseline.cpp 
  baselines.cpp 
  centroidmethod.cpp 
  centroidprocess.cpp 
  chemicalformula.cpp 
  chromatogram.cpp 
  controlmethod.cpp 
  datafile.cpp 
  datafilebroker.cpp 
  datainterpreter.cpp 
  datapublisher.cpp 
  datasubscriber.cpp 
  description.cpp 
  descriptions.cpp 
  elementalcomposition.cpp 
  elementalcompositioncollection.cpp 
  elementalcompositionmethod.cpp
  error_code.cpp
  isotopecluster.cpp 
  isotopemethod.cpp 
  lcmsdataset.cpp 
  lockmass.cpp
  mappedspectrum.cpp
  mappedspectrum.hpp
  mappedspectra.cpp
  mappedspectra.hpp
  mappedimage.cpp
  mappedimage.hpp      
  massspectrometer.cpp 
  massspectrometerbroker.cpp 
  massspectrometer_factory.cpp 
  massspectra.cpp 
  massspectrum.cpp 
  molecule.cpp 
  msassignedmass.cpp 
  mscalibratemethod.cpp 
  mscalibrateresult.cpp 
  mscalibration.cpp 
  mswatchions.cpp 
  mslockmethod.cpp 
  mspeakinfoitem.cpp 
  msproperty.cpp 
  msreference.cpp 
  msreferencedefns.cpp 
  msreferences.cpp
  msident.cpp 
  msquant.cpp 
  peak.cpp 
  peakasymmetry.cpp 
  peakmethod.cpp 
  peakresolution.cpp 
  peaks.cpp 
  processeddataset.cpp 
  processmethod.cpp 
  reportmethod.cpp 
  retentiontime.cpp
  samplerun.cpp
  scanlaw.hpp
  tableofelement.cpp 
  targetingmethod.cpp
  theoreticalplate.cpp
  threshold_method.cpp
  timeutil.cpp 
  trace.cpp 
  traceaccessor.cpp 
  visitor.cpp 
  ctable.cpp 
  ctfile.cpp 
  peakresult.cpp 
  waveform.cpp 
  annotation.cpp 
  annotations.cpp 
  mspeakinfo.cpp
  mspeakinfo.hpp   
  mspeak.cpp 
  mspeaks.cpp 
  msqpeak.cpp 
  msqpeaks.cpp
  mschromatogramextractor.cpp     
  mschromatogrammethod.cpp
  mssimulatormethod.cpp
  moltable.cpp
  moltable.hpp
  spectrogram.cpp
  spectrogram.hpp   
  targeting.cpp
  targeting.hpp  
  logging_hook.cpp
  logging_hook.hpp   
  profile_time_array.cpp
  profile_time_array.hpp  
  quanmethod.cpp 
  quancompounds.cpp 
  quancompound.cpp 
  quansequence.cpp 
  quansample.cpp 
  quanreport.cpp 
  quanresponse.cpp 
  idaudit.cpp 
  msfinder.cpp 
  quanresponses.cpp 
  quancalibration.cpp 
  quancalibrations.cpp 
  serializer.cpp )

target_include_directories( adcontrols PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )
target_link_libraries( adcontrols LINK_PUBLIC ${Boost_LIBRARIES} adportable )

#set_target_properties( adcontrols  PROPERTIES  LIBRARY_OUTPUT_DIRECTORY ${QTPLATZ_BINARY_DIR}/lib/qtplatz )      

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT libraries 
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )

install( FILES
  acceptor.hpp 
  acquireddataset.hpp 
  adcontrols.hpp 
  baseline.hpp 
  baselines.hpp 
  centroidmethod.hpp 
  centroidprocess.hpp 
  chemicalformula.hpp 
  chromatogram.hpp 
  controlmethod.hpp 
  datafile.hpp 
  datafilebroker.hpp 
  datainterpreter.hpp 
  datapublisher.hpp 
  datasubscriber.hpp 
  description.hpp 
  descriptions.hpp 
  elementalcomposition.hpp 
  elementalcompositioncollection.hpp 
  elementalcompositionmethod.hpp
  error_code.hpp
  isotopecluster.hpp 
  isotopemethod.hpp 
  lcmsdataset.hpp 
  lockmass.hpp
  mappedspectrum.hpp
  mappedspectrum.hpp
  mappedspectra.hpp
  mappedspectra.hpp
  mappedimage.hpp
  mappedimage.hpp      
  massspectrometer.hpp 
  massspectrometerbroker.hpp 
  massspectrometer_factory.hpp 
  massspectra.hpp 
  massspectrum.hpp 
  molecule.hpp 
  msassignedmass.hpp 
  mscalibratemethod.hpp 
  mscalibrateresult.hpp 
  mscalibration.hpp 
  mswatchions.hpp 
  mslockmethod.hpp 
  mspeakinfoitem.hpp 
  msproperty.hpp 
  msreference.hpp 
  msreferencedefns.hpp 
  msreferences.hpp 
  msident.hpp 
  msquant.hpp 
  peak.hpp 
  peakasymmetry.hpp 
  peakmethod.hpp 
  peakresolution.hpp 
  peaks.hpp 
  processeddataset.hpp 
  processmethod.hpp 
  reportmethod.hpp 
  retentiontime.hpp
  samplerun.hpp
  scanlaw.hpp
  tableofelement.hpp 
  targetingmethod.hpp
  theoreticalplate.hpp
  threshold_method.hpp
  timeutil.hpp 
  trace.hpp 
  traceaccessor.hpp 
  visitor.hpp 
  ctable.hpp 
  ctfile.hpp 
  peakresult.hpp 
  waveform.hpp 
  annotation.hpp 
  annotations.hpp 
  mspeakinfo.hpp 
  mspeak.hpp 
  mspeaks.hpp 
  msqpeak.hpp 
  msqpeaks.hpp
  mschromatogramextractor.hpp
  mschromatogrammethod.hpp
  mssimulatormethod.hpp
  moltable.hpp
  spectrogram.hpp 
  targeting.hpp 
  logging_hook.hpp 
  profile_time_array.hpp 
  quanmethod.hpp 
  quancompounds.hpp 
  quancompound.hpp 
  quansequence.hpp 
  quansample.hpp 
  quanreport.hpp 
  quanresponse.hpp 
  idaudit.hpp 
  msfinder.hpp 
  quanresponses.hpp 
  quancalibration.hpp 
  quancalibrations.hpp 
  serializer.hpp
  DESTINATION include/adcontrols COMPONENT headers )
