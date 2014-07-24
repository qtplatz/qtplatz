#-------------------------------------------------
#
# Project created by QtCreator 2010-06-26T16:45:23
#
#-------------------------------------------------

QT       -= core gui 

TARGET = adcontrols
TEMPLATE = lib

include(../../qtplatzlibrary.pri)
include(../../boost.pri)
LIBS += -l$$qtLibraryTarget(adportable)

!win32 {
  LIBS += -lboost_date_time -lboost_system -lboost_wserialization -lboost_serialization 
}

DEFINES += ADCONTROLS_LIBRARY

SOURCES += acceptor.cpp \
    acquireddataset.cpp \
    adcontrols.cpp \
    baseline.cpp \
    baselines.cpp \
    centroidmethod.cpp \
    centroidprocess.cpp \
    chemicalformula.cpp \
    chromatogram.cpp \
    controlmethod.cpp \
    datafile.cpp \
    datafilebroker.cpp \
    datainterpreter.cpp \
    datapublisher.cpp \
    datasubscriber.cpp \
    description.cpp \
    descriptions.cpp \
    element.cpp \
    elementalcomposition.cpp \
    elementalcompositioncollection.cpp \
    elementalcompositionmethod.cpp \
    elements.cpp \
    isotopecluster.cpp \
    isotopemethod.cpp \
    lcmsdataset.cpp \
    lockmass.cpp \
    massspectrometer.cpp \
    massspectrometerbroker.cpp \
    massspectrometer_factory.cpp \
    massspectra.cpp \
    massspectrum.cpp \
    msassignedmass.cpp \
    mscalibratemethod.cpp \
    mscalibrateresult.cpp \
    mscalibration.cpp \
    mswatchions.cpp \
    mslockmethod.cpp \
    mspeakinfoitem.cpp \
    msproperty.cpp \
    msreference.cpp \
    msreferencedefns.cpp \
    msreferences.cpp \
    msident.cpp \
    msquant.cpp \
    peak.cpp \
    peakasymmetry.cpp \
    peakmethod.cpp \
    peakresolution.cpp \
    peaks.cpp \
    processeddataset.cpp \
    processmethod.cpp \
    reportmethod.cpp \
    tableofelement.cpp \
    targetingmethod.cpp \
    theoreticalplate.cpp \
    timeutil.cpp \
    trace.cpp \
    traceaccessor.cpp \
    visitor.cpp \
    ctable.cpp \
    ctfile.cpp \
    peakresult.cpp \
    waveform.cpp \
    annotation.cpp \
    annotations.cpp \
    mspeakinfo.cpp \
    mspeak.cpp \
    mspeaks.cpp \
    msqpeak.cpp \
    msqpeaks.cpp \
    mschromatogrammethod.cpp \
    spectrogram.cpp \
    targeting.cpp \
    logging_hook.cpp \
    profile_time_array.cpp \
    quanmethod.cpp \
    quancompounds.cpp \
    quancompound.cpp \
    quansequence.cpp \
    quansample.cpp \
    quanreport.cpp \
    quanresponse.cpp \
    idaudit.cpp \
    msfinder.cpp

HEADERS += acceptor.hpp \
    acquireddataset.hpp \
    adcontrols.hpp \
    adcontrols_global.h \
    baseline.hpp \
    baselines.hpp \
    centroidmethod.hpp \
    centroidprocess.hpp \
    chemicalformula.hpp \
    chromatogram.hpp \
    controlmethod.hpp \
    datafile.hpp \
    datafile_factory.hpp \
    datafilebroker.hpp \
    datainterpreter.hpp \
    datapublisher.hpp \
    datasubscriber.hpp \
    description.hpp \
    descriptions.hpp \
    element.hpp \
    elementalcomposition.hpp \
    elementalcompositioncollection.hpp \
    elementalcompositionmethod.hpp \
    elements.hpp \
    isotopecluster.hpp \
    isotopemethod.hpp \
    lcmsdataset.hpp \
    lockmass.hpp \
    massspectrometer.hpp \
    massspectrometerbroker.hpp \
    massspectrometer_factory.hpp \
    massspectra.hpp \
    massspectrum.hpp \
    msassignedmass.hpp \
    mscalibratemethod.hpp \
    mscalibrateresult.hpp \
    mscalibration.hpp \
    mswatchions.hpp \
    mslockmethod.hpp \
    mspeakinfoitem.hpp \
    msproperty.hpp \
    msreference.hpp \
    msreferencedefns.hpp \
    msreferences.hpp \
    msident.hpp \
    msquant.hpp \
    peak.hpp \
    peakasymmetry.hpp \
    peakmethod.hpp \
    peakresolution.hpp \
    peaks.hpp \
    processeddataset.hpp \
    processmethod.hpp \
    reportmethod.hpp \
    tableofelement.hpp \
    targetingmethod.hpp \
    theoreticalplate.hpp \
    timeutil.hpp \
    trace.hpp \
    traceaccessor.hpp \
    visitor.hpp \
    ctable.hpp \
    ctfile.hpp \
    peakresult.hpp \
    waveform.hpp \
    annotation.hpp \
    annotations.hpp \
    molecule.hpp \
    isotopes.hpp \
    mspeakinfo.hpp \
    mspeak.hpp \
    mspeaks.hpp \
    msqpeak.hpp \
    msqpeaks.hpp \
    mschromatogrammethod.hpp \
    spectrogram.hpp \
    targeting.hpp \
    logging_hook.hpp \
    controlmethodaccerror.hpp \
    profile_time_array.hpp \
    quanmethod.hpp \
    quancompounds.hpp \
    quancompound.hpp \
    quansequence.hpp \
    quansample.hpp \
    quanreport.hpp \
    quanresponse.hpp \
    idaudit.hpp \
    msfinder.hpp

!win32: HEADERS += metric/prefix.hpp

OTHER_FILES += \
    adcontrols.pri
