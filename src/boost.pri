win32 {
    BOOST = $$(BOOST_ROOT)
    isEmpty( BOOST ) {
      BOOST = C:/Boost
    }
    INCLUDEPATH += $${BOOST}/include/boost-1_51
    LIBS += -L$${BOOST}/lib
} else {
    INCLUDEPATH += /usr/local/boost-1.51/include
    LIBS += -L/usr/local/boost-1.51/lib
}
