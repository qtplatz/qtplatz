ACE_ROOT = $$(ACE_ROOT)
TAO_ROOT = $$(TAO_ROOT)

isEmpty( ACE_ROOT ) {
  win32 { 
    ACE_ROOT = C:/ACE_wrappers
    TAO_ROOT = $${ACE_ROOT}
  } else {
    ACE_ROOT = /usr/local/ace+tao/6.1.8
    TAO_ROOT = $${ACE_ROOT}
  }
}

win32 {
      INCLUDEPATH *= $${ACE_ROOT}
      INCLUDEPATH *= $${TAO_ROOT}
      LIBS += -L$${ACE_ROOT}/lib
} else {
      INCLUDEPATH *= $${ACE_ROOT}/include
      LIBS *= -L$${ACE_ROOT}/lib
}

message( "using ace+tao " $${ACE_ROOT} $${TAO_ROOT} )
