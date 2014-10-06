# ACE+TAO

include ( config.pri )

ACE_ROOT = $$(ACE_ROOT)
TAO_ROOT = $$(TAO_ROOT)

isEmpty( ACE_ROOT ) {
  win32: ACE_ROOT=C:/ACE_wrapper
  else: ACE_ROOT=/usr/local/ace+tao/$${ACE_VERSION}
  message("empty ACE_ROOT, using default: " $$(ACE_ROOT))
}
isEmpty( TAO_ROOT ): TAO_ROOT=$${ACE_ROOT}

win32 {
      INCLUDEPATH *= $${ACE_ROOT}
      INCLUDEPATH *= $${TAO_ROOT}
      LIBS += -L$${ACE_ROOT}\\lib
} else {
      INCLUDEPATH *= $${ACE_ROOT}/include $${ACE_ROOT} $${TAO_ROOT}
      LIBS *= -L$${ACE_ROOT}/lib
}

message( "using ace+tao " $${ACE_ROOT} $${TAO_ROOT} )
