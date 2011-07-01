#-------------------------------------------------
#
# Project created by QtCreator 2011-06-10T10:44:33
#
#-------------------------------------------------

QT       += core gui declarative

TARGET = signalsampling
TEMPLATE = app
INCLUDEPATH += $$(BOOST_INCLUDE) ../../src/libs
LIBS += -L../../lib/qtPlatz -ladcontrolsd
include(../../src/boost.pri)

mac {
  QWT = /usr/local/qwt-6.0.1-svn
  INCLUDEPATH += /usr/local/qwt-6.0.1-svn/lib/qwt.framework/Headers
  LIBS += -L$$QWT/lib -lqwt_debug
  QMAKE_LFLAGS += -F/usr/local/qwt-6.0.1-svn/lib
} else {
  INCLUDEPATH += $$(QWT)/include
  LIBS += -L$$(QWT)/lib -l$$qtLibraryTarget(qwt)
}

#LIBS += -framework /usr/local/qwt-6.0.1-svn/lib/qwt.framework
mac {
    copy2build.input = OTHER_FILES
    copy2build.output = $${OUT_PWD}/${QMAKE_TARGET}.app/Contents/Resources/${QMAKE_FILE_BASE}.qml
    copy2build.commands = $$QMAKE_COPY ${QMAKE_FILE_IN} ${QMAKE_FILE_OUT}
    copy2build.name = COPY ${QMAKE_FILE_IN}
    copy2build.CONFIG += no_link
    QMAKE_EXTRA_COMPILERS += copy2build
}

QMLFILES += bezel.qml \
            qml/ProcessMethodEditor.qml \
            qml/content/BusyIndicator.qml \
            qml/content/CategoryDelegate.qml \
            qml/content/ScrollBar.qml

SOURCES += main.cpp\
        mainwindow.cpp \
    plot.cpp \
    centroidmethodmodel.cpp \
    mscalibratemethodmodel.cpp

HEADERS  += mainwindow.hpp \
    plot.hpp \
    centroidmethodmodel.hpp \
    mscalibratemethodmodel.hpp

OTHER_FILES += \
        qml/ProcessMethodEditor.qml \
        qml/content/BusyIndicator.qml \
        qml/content/CategoryDelegate.qml \
        qml/content/ScrollBar.qml \
        qml/content/EditCentroidMethod.qml \
        qml/content/EditMSCalibMethod.qml \
        qml/content/EditElementalCompMethod.qml \
        qml/content/EditIsotopeMethod.qml \
        qml/content/EditTargetMethod.qml \
    qml/content/EditLockMassMethod.qml \
    qml/content/EditIntegrationMethod.qml \
    qml/content/EditPeakIdTable.qml \
    qml/content/EditReportMethod.qml \
    qml/content/EditTextItem.qml \
    qml/content/ScanType.qml \
    qml/content/ScanTypeDetails.qml \
    qml/content/TextInputBox.qml \
    qml/content/CaptionText.qml

RESOURCES += \
    resources.qrc
