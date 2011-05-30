win32 {
      INCLUDEPATH *= $$(QWT)/include
} else {
      QWT = /usr/local/qwt-6.0.1-svn
      INCLUDEPATH *= $$(QWT)/include
}
