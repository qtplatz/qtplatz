
CONFIG(debug, debug|release) {
  DEFINES += DEBUG
}

BOOST_VERSION=boost-1_53
ACE_VERSION=6.1.8
QWT_VERSION=6.0.3-svn

# does not override if environment variable already exist

# ACE+TAO
isEmpty( $$ACE_ROOT ) {
  win32: ACE_ROOT=C:/ACE_wrapper
  else: ACE_ROOT=/usr/local/ace+tao/$${ACE_VERSION}
}
isEmpty( $$TAO_ROOT ): TAO_ROOT=$$ACE_ROOT

# qwt
QWT = $$QWT
isEmpty( QWT ) {
  win32: QWT= C:/Qwt-$${QWT_VERSION}
  macx|linux-*: QWT=/usr/local/qwt-$${QWT_VERSION}	 
}

QTPLATZ_CONFIG += Acquire
#QTPLATZ_CONFIG += Sequence
#QTPLATZ_CONFIG += Dataproc
#QTPLATZ_CONFIG += ChemSpider
#QTPLATZ_CONFIG += Chemistry

# no chemistry for ARM platform
linux-arm-*: QTPLATZ_CONFIG -= ChemSpider Chemistry
