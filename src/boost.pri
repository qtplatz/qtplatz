
include( config.pri )

BOOST_ROOT=$$(BOOST_ROOT)

isEmpty( BOOST_ROOT ) {
    win32: BOOST_ROOT=C:/Boost
    else:  BOOST_ROOT=/usr/local/boost-1_57
}

win32 {

    BOOST_INCLUDE = $$(BOOST_INCLUDE)
    isEmpty( BOOST_INCLUDE ): BOOST_INCLUDE = $${BOOST_ROOT}/include/boost-1_57

    BOOST_LIBRARY = $$(BOOST_LIBRARY)
    isEmpty( BOOST_LIBRARY ): BOOST_LIBRARY = $${BOOST_ROOT}\\lib

    LIBS *= -L$${BOOST_LIBRARY}

} else {
    BOOST_INCLUDE = $${BOOST_ROOT}/include
    BOOST_LIBRARY = $${BOOST_ROOT}/lib
    QMAKE_LFLAGS *= -L$${BOOST_LIBRARY}
}

INCLUDEPATH *= $${BOOST_INCLUDE}
#message( "using boost: " $${INCLUDEPATH})



