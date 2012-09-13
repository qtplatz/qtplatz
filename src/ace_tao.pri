win32 {
      INCLUDEPATH *= $$(ACE_ROOT)
      INCLUDEPATH *= $$(TAO_ROOT)
} else {
      INCLUDEPATH *= $$(ACE_ROOT)/include
      LIBS += -L$$(ACE_ROOT)/lib
}
