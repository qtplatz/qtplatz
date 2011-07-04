#-------------------------------------------------
#
# Project created by QtCreator 2011-07-04T16:13:36
#
#-------------------------------------------------

QT       += core gui declarative

TARGET = signalsampling
TEMPLATE = app
INCLUDEPATH += $$(BOOST_INCLUDE)

macx {
  QWT = /usr/local/qwt-6.0.1-svn
  INCLUDEPATH += /usr/local/qwt-6.0.1-svn/lib/qwt.framework/Headers
  LIBS += -L$$QWT/lib -lqwt_debug
  DESTDIR = ../../bin
} else {
  INCLUDEPATH += $$(QWT)/include
  LIBS += -L$$(QWT)/lib -l$$qtLibraryTarget(qwt)
}

SOURCES += main.cpp\
        mainwindow.cpp \
    plot.cpp

HEADERS  += mainwindow.hpp \
    plot.hpp

FORMS    +=

OTHER_FILES += \
    qml/application.qml

RESOURCES += \
    resources.qrc
