# ACE+TAO

include ( config.pri )

ACE_ROOT = $$(ACE_ROOT)
TAO_ROOT = $$(TAO_ROOT)

win32 {
      INCLUDEPATH *= $${ACE_ROOT}
      INCLUDEPATH *= $${TAO_ROOT}
      LIBS += -L$${ACE_ROOT}\\lib
} else {
      INCLUDEPATH *= $${ACE_ROOT}/include $${ACE_ROOT} $${TAO_ROOT}
      LIBS *= -L$${ACE_ROOT}/lib
}

message( "using ace+tao " $${ACE_ROOT} $${TAO_ROOT} )
