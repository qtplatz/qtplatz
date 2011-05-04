win32 {
    BOOST = $$(BOOST_ROOT)
    isEmpty( BOOST ) {
      BOOST = C:/Boost
    }
    INCLUDEPATH += $${BOOST}/include/boost-1_46
    LIBS += -L$${BOOST}/lib
} else {
    INCLUDEPATH += /usr/local/include
}
