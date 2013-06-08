
CONFIG(debug, debug|release) {
  DEFINES += DEBUG
}

win32 {
    DEFINES += _WIN32_WINNT=0x0700
} else {
    QMAKE_CXXFLAGS *= -std=c++11
}
macx {
    QMAKE_CXXFLAGS *= -std=c++11
}

BOOST_VERSION=boost-1_53
ACE_VERSION=6.1.8
QWT_VERSION=6.1.0-svn

greaterThan( QT_MAJOR_VERSION, 4 ): QWT_VERSION=$${QWT_VERSION}-qt$$QT_MAJOR_VERSION

# does not override if environment variable already exist

# ACE+TAO
isEmpty( $$ACE_ROOT ) {
  win32: ACE_ROOT=C:/ACE_wrapper
  else: ACE_ROOT=/usr/local/ace+tao/$${ACE_VERSION}
}
isEmpty( $$TAO_ROOT ): TAO_ROOT=$$ACE_ROOT

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
#QTPLATZ_CONFIG += ChemSpider
QTPLATZ_CONFIG += Chemistry

# no chemistry for ARM platform
linux-arm-*: QTPLATZ_CONFIG -= ChemSpider Chemistry
