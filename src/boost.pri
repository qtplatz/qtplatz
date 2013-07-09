
include( config.pri )




BOOST_ROOT=$$(BOOST_ROOT)
isEmpty( BOOST_ROOT ) {
    win32: BOOST_ROOT=C:/Boost
    else:  BOOST_ROOT=/usr/local/$${BOOST_VERSION}
}

win32 {
      BOOST_INCLUDE = $${BOOST_ROOT}/include/$${BOOST_VERSION}
      LIBS *= -L$${BOOST_ROOT}\\lib
} else {
      BOOST_INCLUDE = $${BOOST_ROOT}/include
      QMAKE_LFLAGS *= -L$${BOOST_ROOT}/lib
}

INCLUDEPATH += $${BOOST_INCLUDE}

# message( "using boost " $${BOOST_INCLUDE} )
