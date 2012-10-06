win32 {
      INCLUDEPATH *= $$(ACE_ROOT)
      INCLUDEPATH *= $$(TAO_ROOT)
} else {
      INCLUDEPATH *= $$(ACE_ROOT)/include
      QMAKE_LFLAGS += -L$$(ACE_ROOT)/lib -R$$(ACE_ROOT)/lib
}
