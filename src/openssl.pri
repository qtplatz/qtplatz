win32 {
    OPENSSL = $$(OPENSSL_ROOT)
    isEmpty( OPENSSL ) {
      OPENSSL = C:/openssl
    }
    INCLUDEPATH += $${OPENSSL}/include
    LIBS += -L$${OPENSSL}/lib -lssleay32 -llibeay32
} else {
    INCLUDEPATH += /usr/local/include
    LIBS += -L/usr/local/lib
}
