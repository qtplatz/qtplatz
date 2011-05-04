INCLUDEPATH += $$(ACE_ROOT) $$(TAO_ROOT)
LIBS *= -L$$(ACE_ROOT)/lib
CONFIG(debug, debug|release) : LIBS *= -lACEd
CONFIG(release, debug|release) : LIBS *= -lACE
