#-------------------------------------------------
#
# Project created by QtCreator 2010-06-26T16:45:23
#
#-------------------------------------------------

QT       -= gui

TARGET = adcontrols
TEMPLATE = lib
INCLUDEPATH += $$(SATOOLS_ROOT)/bin

include(../../adilibrary.pri)
include(../../boost.pri)
include(../acewrapper/acewrapper.pri)

DEFINES += ADCONTROLS_LIBRARY

SOURCES += massspectrum.cpp \
    description.cpp \
    descriptions.cpp \
    tableofelements.cpp \
    adcontrols.cpp \
    chromatogram.cpp \
    element.cpp \
    elements.cpp \
    mslockmethod.cpp \
    cpeakmethod.cpp \
    mscalibratemethod.cpp \
    mscalibraterefdefns.cpp \
    elementalcompositionmethod.cpp \
    isotopemethod.cpp \
    reportmethod.cpp \
    targetingmethod.cpp \
    dataproviderbroker.cpp \
    datasubscriber.cpp \
    datapublisher.cpp \
    lcmsdataset.cpp \
    acquireddataset.cpp \
    processeddataset.cpp \
    mscalibration.cpp \
    elementalcomposition.cpp \
    elementalcompositioncollection.cpp \
    lockmass.cpp \
    processmethod.cpp \
    msproperty.cpp \
    processresult.cpp \
    peak.cpp \
    peaks.cpp \
    baseline.cpp \
    baselines.cpp \
    timeutil.cpp \
    theoreticalplate.cpp \
    peakasymmetry.cpp \
    peakresolution.cpp \
    traceaccessor.cpp \
    trace.cpp \
    msreference.cpp \
    msreferences.cpp \
    isocluster.cpp \
    samassspectrum.cpp \
    mscalibrateresult.cpp

HEADERS += massspectrum.h\
        adcontrols_global.h \
    import_sacontrols.h \
    adcontrols.h \
    description.h \
    descriptions.h \
    tableofelements.h \
    chromatogram.h \
    element.h \
    elements.h \
    mslockmethod.h \
    cpeakmethod.h \
    mscalibratemethod.h \
    mscalibraterefdefns.h \
    elementalcompositionmethod.h \
    isotopemethod.h \
    reportmethod.h \
    targetingmethod.h \
    dataproviderbroker.h \
    datasubscriber.h \
    datapublisher.h \
    lcmsdataset.h \
    acquireddataset.h \
    processeddataset.h \
    mscalibration.h \
    elementalcomposition.h \
    elementalcompositioncollection.h \
    lockmass.h \
    processmethod.h \
    msproperty.h \
    processresult.h \
    cpeak.h \
    cpeaks.h \
    cbaseline.h \
    cbaselines.h \
    timeutil.h \
    theoreticalplate.h \
    peakasymmetry.h \
    peakresolution.h \
    traceaccessor.h \
    trace.h \
    msreference.h \
    msreferences.h \
    isocluster.h \
    samassspectrum.h \
    mscalibrateresult.h

OTHER_FILES += \
    adcontrols.pri
