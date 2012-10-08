win32 {
      INCLUDEPATH *= $$(ACE_ROOT)
      INCLUDEPATH *= $$(TAO_ROOT)
      LIBS += -L$$(ACE_ROOT)\\lib
} else {
      INCLUDEPATH *= $$(ACE_ROOT)/include
      QMAKE_LFLAGS += -L$$(ACE_ROOT)/lib
}
