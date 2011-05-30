win32 {
      INCLUDEPATH *= $$(ACE_ROOT)
      INCLUDEPATH *= $$(TAO_ROOT)
} else {
      INCLUDEPATH *= /usr/local/ace
      INCLUDEPATH *= /usr/local/tao
}
