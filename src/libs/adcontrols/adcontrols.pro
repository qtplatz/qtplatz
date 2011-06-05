#-------------------------------------------------
#
# Project created by QtCreator 2010-06-26T16:45:23
#
#-------------------------------------------------

QT       -= gui 

TARGET = adcontrols
TEMPLATE = lib

include(../../qtplatz_library.pri)
include(../../boost.pri)
include(../acewrapper/acewrapper_dependencies.pri)
LIBS *= -l$$qtLibraryTarget(acewrapper)
LIBS *= -lACE -ladportable
LIBS *= -lboost_serialization -lboost_wserialization

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
    msassignedmass.hpp \
    mscalibratemethod.hpp \
    mscalibrateresult.hpp \
    mscalibration.hpp \
    mslockmethod.hpp \
    mspeakinfoitem.hpp \
    msproperty.hpp \
    msreference.hpp \
    msreferencedefns.hpp \
    msreferences.hpp \
    peak.hpp \
    peakasymmetry.hpp \
    peakresolution.hpp \
    peaks.hpp \
    processeddataset.hpp \
    processmethod.hpp \
    reportmethod.hpp \
    tableofelements.hpp \
    targetingmethod.hpp \
    theoreticalplate.hpp \
    timeutil.hpp \
    trace.hpp \
    traceaccessor.hpp \
    visitor.hpp

OTHER_FILES += \
    adcontrols.pri
