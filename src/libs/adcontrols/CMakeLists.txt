
#  LIBS += -lboost_date_time -lboost_system -lboost_wserialization -lboost_serialization 

cmake_policy(SET CMP0022 NEW)

set(Boost_USE_STATIC_LIBS ON)

find_package(Boost 1.56 COMPONENTS date_time system wserialization serialization)

add_definitions(-DADCONTROLS_LIBRARY)

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
  isotopecluster.cpp 
  isotopemethod.cpp 
  lcmsdataset.cpp 
  lockmass.cpp 
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
  tableofelement.cpp 
  targetingmethod.cpp 
  theoreticalplate.cpp 
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
  mspeak.cpp 
  mspeaks.cpp 
  msqpeak.cpp 
  msqpeaks.cpp 
  mschromatogrammethod.cpp 
  spectrogram.cpp 
  targeting.cpp 
  logging_hook.cpp 
  profile_time_array.cpp 
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

