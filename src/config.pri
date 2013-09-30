
CONFIG(debug, debug|release) {
  DEFINES += DEBUG
}
#DEFINES += BOOST_REGEX

win32 {
    DEFINES += _WIN32_WINNT=0x0700
} else {
    QMAKE_CXXFLAGS *= -std=c++11
}
macx {
     LIBS += -stdlib=libc++
     QMAKE_CXXFLAGS *= -std=c++11
     QMAKE_CXXFLAGS *= -stdlib=libc++
     QMAKE_CXXFLAGS *= -ftemplate-depth=256
     QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.7
}
*-g++* {
    QMAKE_CXXFLAGS *= -O2
}

BOOST_VERSION=boost-1_54
ACE_VERSION=6.2.1
QWT_VERSION=6.1.1-svn

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
#QTPLATZ_CONFIG += ExampleTOF
#!macx: QTPLATZ_CONFIG += Chemistry

# no chemistry for ARM platform
linux-arm-*: QTPLATZ_CONFIG -= ChemSpider Chemistry
