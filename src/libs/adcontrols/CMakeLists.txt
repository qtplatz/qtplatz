cmake_policy( SET CMP0028 NEW )
cmake_policy( SET CMP0022 NEW )

project( adcontrols )

add_definitions(-DADCONTROLS_LIBRARY)
if ( MSVC )
  add_definitions(-D_SCL_SECURE_NO_WARNINGS)
  add_definitions( -wd4251 -wd4244 )
elseif( CMAKE_CXX_COMPILER_ID STREQUAL "GNU" )
  add_definitions(-Wno-attributes)
endif()

add_library( adcontrols SHARED
  adcv/contoursmethod.cpp
  adcv/contoursmethod.hpp
  acceptor.cpp  acceptor.hpp
  acquireddataset.cpp  acquireddataset.hpp
  adcontrols.cpp  adcontrols.hpp
  annotation.cpp  annotation.hpp
  annotations.cpp  annotations.hpp
  axis.hpp  baseline.cpp
  baseline.hpp  baselines.cpp
  baselines.hpp  centroidmethod.cpp
  centroidmethod.hpp  centroidprocess.cpp
  centroidprocess.hpp  chemicalformula.cpp
  chemicalformula.hpp  chromatogram.cpp
  chromatogram.hpp  constants.hpp
  constants.cpp  controlmethod.cpp
  controlmethod.hpp  controlmethod/timedevent.hpp
  controlmethod/timedevent.cpp  controlmethod/timedevents.hpp
  controlmethod/timedevents.cpp  controlmethod/modulecap.hpp
  controlmethod/modulecap.cpp  controlmethod/eventcap.cpp
  controlmethod/eventcap.hpp  controlmethod/tofchromatogrammethod.cpp
  controlmethod/tofchromatogrammethod.hpp  controlmethod/tofchromatogramsmethod.cpp
  controlmethod/tofchromatogramsmethod.hpp  controlmethod/xchromatogramsmethod.cpp
  controlmethod/xchromatogramsmethod.hpp  countingdata.cpp
  countingdata.hpp  countingmethod.cpp
  countingmethod.hpp  countingresult.cpp
  countingresult.hpp  countinghistogram.cpp
  countinghistogram.hpp
  ctable.cpp  ctable.hpp
  ctfile.cpp  ctfile.hpp
  datafile.cpp  datafile.hpp
  datafilebroker.cpp  datafilebroker.hpp
  datainterpreter.cpp  datainterpreter.hpp
  datainterpreterbroker.cpp  datainterpreterbroker.hpp
  datapublisher.cpp  datapublisher.hpp
  datareader.cpp  datareader.hpp
  datasubscriber.cpp  datasubscriber.hpp
  description.cpp  description.hpp
  descriptions.cpp  descriptions.hpp
  elementalcomposition.cpp  elementalcomposition.hpp
  elementalcompositioncollection.cpp  elementalcompositioncollection.hpp
  elementalcompositionmethod.cpp  elementalcompositionmethod.hpp
  error_code.cpp  error_code.hpp
  histogram.cpp  histogram.hpp
  ionreactionmethod.cpp ionreactionmethod.hpp
  isocluster.cpp
  isocluster.hpp
  isotopecluster.cpp
  isotopecluster.hpp
  isotopemethod.cpp  isotopemethod.hpp
  idaudit.cpp
  idaudit.hpp
  lapfinder.cpp
  lapfinder.hpp
  lcmsdataset.cpp
  lcmsdataset.hpp
  lockmass.cpp
  lockmass.hpp
  logging_hook.cpp
  logging_hook.hpp
  make_combination.cpp make_combination.hpp
  mappeddataframe.cpp
  mappeddataframe.hpp
  mappedimage.cpp
  mappedimage.hpp
  mappedspectra.cpp
  mappedspectra.hpp
  mappedspectrum.cpp
  mappedspectrum.hpp
  massspectrometer.cpp
  massspectrometer.hpp
  massspectrometerbroker.cpp
  massspectrometerbroker.hpp
  massspectrometer_factory.cpp
  massspectrometer_factory.hpp
  massspectra.cpp
  massspectra.hpp
  massspectrum.cpp
  massspectrum.hpp
  metidmethod.cpp
  metidmethod.hpp
  molecule.cpp
  molecule.hpp
  msassignedmass.cpp
  msassignedmass.hpp
  mscalibratemethod.cpp
  mscalibratemethod.hpp
  mscalibrateresult.cpp
  mscalibrateresult.hpp
  mscalibration.cpp
  mscalibration.hpp
  msfractuation.cpp
  msfractuation.hpp
  msmoltable.cpp
  msmoltable.hpp
  mslockmethod.cpp
  mslockmethod.hpp
  mspeakinfoitem.cpp
  mspeakinfoitem.hpp
  msproperty.cpp
  msproperty.hpp
  msreference.cpp
  msreference.hpp
  msreferences.cpp
  msreferences.hpp
  msident.cpp
  msident.hpp
  msquant.cpp
  msquant.hpp
  mspeakinfo.cpp
  mspeakinfo.hpp
  mspeak.cpp
  mspeak.hpp
  mspeaks.cpp
  mspeaks.hpp
  msqpeak.cpp
  msqpeak.hpp
  msqpeaks.cpp
  msqpeaks.hpp
  mschromatogrammethod.cpp
  mschromatogrammethod.hpp
  mssimulatormethod.cpp
  mssimulatormethod.hpp
  moltable.cpp
  moltable.hpp
  peak.cpp
  peak.hpp
  peakasymmetry.cpp
  peakasymmetry.hpp
  peakmethod.cpp
  peakmethod.hpp
  peakresolution.cpp
  peakresolution.hpp
  peaks.cpp
  peaks.hpp
  processeddataset.cpp
  processeddataset.hpp
  processmethod.cpp
  processmethod.hpp
  peakresult.cpp
  peakresult.hpp
  profile_time_array.cpp
  profile_time_array.hpp
  quanmethod.cpp
  quanmethod.hpp
  quancompounds.cpp
  quancompounds.hpp
  quancompound.cpp
  quancompound.hpp
  quanresponsemethod.cpp
  quanresponsemethod.hpp
  quansequence.cpp
  quansequence.hpp
  quansample.cpp
  quansample.hpp
  quanreport.cpp
  quanreport.hpp
  quanresponse.cpp
  quanresponse.hpp
  peakd/ra.cpp peakd/ra.hpp
  reportmethod.cpp
  reportmethod.hpp
  retentiontime.cpp
  retentiontime.hpp
  samplerun.cpp
  samplerun.hpp
  scanlaw.hpp
  segment_wrapper.hpp
  spectrogram.cpp
  spectrogram.hpp
  tableofelement.cpp
  tableofelement.hpp
  targetingmethod.cpp
  targetingmethod.hpp
  theoreticalplate.cpp
  theoreticalplate.hpp
  threshold_method.cpp
  threshold_method.hpp
  threshold_action.cpp
  threshold_action.hpp
  timedigitalmethod.cpp
  timedigitalmethod.hpp
  timedigitalhistogram.cpp
  timedigitalhistogram.hpp
  timeutil.cpp
  timeutil.hpp
  tofprotocol.cpp
  tofprotocol.hpp
  trace.cpp
  trace.hpp
  traceaccessor.cpp
  traceaccessor.hpp
  targeting.cpp
  targeting.hpp
  typelist.hpp
  visitor.cpp
  msfinder.cpp
  quanresponses.cpp
  quanresponses.hpp
  quancalibration.cpp
  quancalibration.hpp
  quancalibrations.cpp
  quancalibrations.hpp
  samplinginfo.cpp
  samplinginfo.hpp
  serializer.cpp
  serializer.hpp
  surface.cpp
  surface.hpp
  waveform_filter.cpp
  waveform_filter.hpp
  quan/extract_by_mols.cpp
  quan/extract_by_mols.hpp
  quan/targeting_candidate.cpp
  quan/targeting_candidate.hpp
  targeting/candidate.cpp
  targeting/candidate.hpp
  targeting/isotope.cpp
  targeting/isotope.hpp
  genchromatogram.cpp
  genchromatogram.hpp
  jcb2009_peakresult.cpp jcb2009_peakresult.hpp
  pugrest.cpp pugrest.hpp
  jstrest.cpp jstrest.hpp
  figsharerest.cpp figsharerest.hpp
  )

target_include_directories( adcontrols PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )

if ( CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS_EQUAL "8.3.0" )
  set (LIBCPPFS "stdc++fs")
endif()
message( STATUS "## LIBCPPFS: " ${LIBCPPFS} )


target_link_libraries(
  adcontrols LINK_PUBLIC
  adportable
  adportable_serializer
  ${Boost_LIBRARIES}
  ${LIBCPPFS}
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
  adcontrols_global.h
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
  ctable.hpp
  ctfile.hpp
  datafile.hpp
  datafilebroker.hpp
  datainterpreter.hpp
  datainterpreterbroker.hpp
  datapublisher.hpp
  datareader.hpp
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
  logging_hook.hpp
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
  metidmethod.hpp
  molecule.hpp
  msassignedmass.hpp
  mscalibratemethod.hpp
  mscalibrateresult.hpp
  mscalibration.hpp
  msmoltable.hpp
  mslockmethod.hpp
  mspeakinfoitem.hpp
  msproperty.hpp
  msreference.hpp
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
  serializer.hpp
  spectrogram.hpp
  tableofelement.hpp
  targeting.hpp
  targetingmethod.hpp
  theoreticalplate.hpp
  threshold_method.hpp
  threshold_action.hpp
  timedigitalmethod.hpp
  timedigitalhistogram.hpp
  timeutil.hpp
  trace.hpp
  traceaccessor.hpp
  visitor.hpp
  peakresult.hpp
  annotation.hpp
  annotations.hpp
  mspeakinfo.hpp
  mspeak.hpp
  mspeaks.hpp
  msqpeak.hpp
  msqpeaks.hpp
  mschromatogrammethod.hpp
  mssimulatormethod.hpp
  moltable.hpp
  profile_time_array.hpp
  quanmethod.hpp
  quancompounds.hpp
  quancompound.hpp
  quansequence.hpp
  quansample.hpp
  quanreport.hpp
  quanresponse.hpp
  msfinder.hpp
  quanresponses.hpp
  quancalibration.hpp
  quancalibrations.hpp
  samplinginfo.hpp
  waveform_filter.hpp
  tofprotocol.hpp
  DESTINATION include/adcontrols COMPONENT headers )

install( FILES
  controlmethod/tofchromatogrammethod.hpp
  controlmethod/tofchromatogramsmethod.hpp
  DESTINATION include/adcontrols/controlmethod COMPONENT headers
  )
