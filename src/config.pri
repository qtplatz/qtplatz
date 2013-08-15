
CONFIG(debug, debug|release) {
  DEFINES += DEBUG
}
DEFINES += BOOST_REGEX

win32 {
    DEFINES += _WIN32_WINNT=0x0700
    CONFIG(debug, debug|release):LIBS *= -L"C:/Program Files (x86)/Visual Leak Detector/lib"
    CONFIG(debug, debug|release):INCLUDEPATH += "C:/Program Files (x86)/Visual Leak Detector/include"
} else {
    QMAKE_CXXFLAGS *= -std=c++11
}
macx {
    QMAKE_CXXFLAGS *= -std=c++11
    QMAKE_CXXFLAGS *= -stdlib=libc++
    QMAKE_CXXFLAGS -= -mmacosx-version-min=10.5
    QMAKE_CXXFLAGS *= -mmacosx-version-min=10.7
    QMAKE_LFLAGS   -= -mmacosx-version-min=10.5
    QMAKE_LFLAGS   *= -mmacosx-version-min=10.7 -stdlib=libc++
}

BOOST_VERSION=boost-1_54
ACE_VERSION=6.2.0
QWT_VERSION=6.1.0-svn-qt$$QT_MAJOR_VERSION

# does not override if environment variable already exist

# ACE+TAO
ACE_ROOT = $$(ACE_ROOT)
TAO_ROOT = $$(TAO_ROOT)
isEmpty( ACE_ROOT ) {
  win32: ACE_ROOT=C:/ACE_wrapper
  else: ACE_ROOT=/usr/local/ace+tao/$${ACE_VERSION}
}
isEmpty( TAO_ROOT ): TAO_ROOT=$${ACE_ROOT}

# qwt
QWT = $$(QWT)
isEmpty( QWT ) {
  win32: QWT= C:/Qwt-$${QWT_VERSION}
  macx|linux-*: QWT=/usr/local/qwt-$${QWT_VERSION}
}

# OpenBabel
OPENBABEL_ROOT = $$(OPENBABEL_ROOT)
isEmpty ( OPENBABEL_ROOT ) {
  win32: OPENBABEL_ROOT=C:/openbabel
  else: OPENBABEL_ROOT=/usr/local
}

QTPLATZ_CONFIG += Acquire
QTPLATZ_CONFIG += Sequence
QTPLATZ_CONFIG += Dataproc
!macx: QTPLATZ_CONFIG += Chemistry

# no chemistry for ARM platform
linux-arm-*: QTPLATZ_CONFIG -= ChemSpider Chemistry
