#-------------------------------------------------
#
# Project created by QtCreator 2010-06-26T16:45:23
#
#-------------------------------------------------

QT       -= gui

TARGET = adcontrols
TEMPLATE = lib

include(../../qtplatz_lib_dynamic.pri)
include(../../boost.pri)
include(../acewrapper/acewrapper.pri)

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
    massspectrum.cpp \
    msassignedmass.cpp \
    mscalibratemethod.cpp \
    mscalibrateresult.cpp \
    mscalibration.cpp \
    mslockmethod.cpp \
    mspeakinfoitem.cpp \
    msproperty.cpp \
    msreference.cpp \
    msreferencedefns.cpp \
    msreferences.cpp \
    peak.cpp \
    peakasymmetry.cpp \
    peakresolution.cpp \
    peaks.cpp \
    processeddataset.cpp \
    processmethod.cpp \
    reportmethod.cpp \
    tableofelements.cpp \
    targetingmethod.cpp \
    theoreticalplate.cpp \
    timeutil.cpp \
    trace.cpp \
    traceaccessor.cpp \
    visitor.cpp

HEADERS += acceptor.h \
    acquireddataset.h \
    adcontrols.h \
    adcontrols_global.h \
    baseline.h \
    baselines.h \
    centroidmethod.h \
    centroidprocess.h \
    chemicalformula.h \
    chromatogram.h \
    datafile.h \
    datafile_factory.h \
    datafilebroker.h \
    datainterpreter.h \
    datapublisher.h \
    datasubscriber.h \
    description.h \
    descriptions.h \
    element.h \
    elementalcomposition.h \
    elementalcompositioncollection.h \
    elementalcompositionmethod.h \
    elements.h \
    isotopecluster.h \
    isotopemethod.h \
    lcmsdataset.h \
    lockmass.h \
    massspectrometer.h \
    msassignedmass.h \
    mscalibratemethod.h \
    mscalibrateresult.h \
    mscalibration.h \
    mslockmethod.h \
    mspeakinfoitem.h \
    msproperty.h \
    msreference.h \
    msreferencedefns.h \
    msreferences.h \
    peak.h \
    peakasymmetry.h \
    peakresolution.h \
    peaks.h \
    processeddataset.h \
    processmethod.h \
    reportmethod.h \
    tableofelements.h \
    targetingmethod.h \
    theoreticalplate.h \
    timeutil.h \
    trace.h \
    traceaccessor.h \
    visitor.h

OTHER_FILES += \
    adcontrols.pri
