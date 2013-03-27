BOOST_VERSION=boost-1_53

BOOST_ROOT=$$(BOOST_ROOT)
isEmpty( BOOST_ROOT ) {
    win32: BOOST_ROOT=C:/Boost
    else:  BOOST_ROOT=/usr/local/$${BOOST_VERSION}
}

win32 {
      INCLUDEPATH *= $${BOOST_ROOT}/include/$${BOOST_VERSION}
      LIBS *= -L$${BOOST_ROOT}\\lib
} else {
      INCLUDEPATH *= $${BOOST_ROOT}/include
      QMAKE_LFLAGS *= -L$${BOOST_ROOT}/lib
}

