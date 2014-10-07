#--------------
# Customer compiler for tao_idl
#
include( config.pri )

for(idl, IDLFILES): PRE_TARGETDEPS += $$replace( idl, ".idl", "C.cpp" )

#vcxproj after vs2012 can not handle command string expanded fro "$${ACE_ROOT}
#that contains <quot></quot> by vc.  nmake does not has such problem though
#Windows always require $ACE_ROOT/lib in path anyway for tao_idl get to work


win32: TAO_IDL = $${ACE_ROOT}\bin\tao_idl
else: TAO_IDL = tao_idl

message( "TAO_IDL" $${TAO_IDL} )

#IDL_FLAGS = -Wb,pre_include=ace/pre.h -Wb,post_include=ace/post.h $${IDL_INCLUDES}
IDL_FLAGS = -Wb,pre_include=workaround/ace/pre.h -Wb,post_include=workaround/ace/post.h $${IDL_INCLUDES}

tao_idlC.CONFIG += no_link
tao_idlC.dependency_type = TYPE_C
tao_idlC.input  = IDLFILES
contains(TEMPLATE, "vc.*") {
  tao_idlC.output = ${QMAKE_FILE_BASE}C.cxx 
} else {
  tao_idlC.output = ${QMAKE_FILE_BASE}C.cpp
}
tao_idlC.clean  = ${QMAKE_FILE_BASE}C.cpp ${QMAKE_FILE_BASE}C.h ${QMAKE_FILE_BASE}C.inl
tao_idlC.commands = $${TAO_IDL} $${IDL_FLAGS} ${QMAKE_FILE_IN}
tao_idlC.variable_out = SOURCES
tao_idlC.name   = TAO_IDL_C ${QMAKE_FILE_IN}
contains(TEMPLATE, "vc.*") {
  tao_idlC.commands += $$escape_expand(\\r\\h) echo -n > $$tao_idlC.output
  tao_idlC.clean += ${QMAKE_FILE_BASE}C.cxx
}

QMAKE_EXTRA_COMPILERS += tao_idlC

!contains(TEMPLATE, "vc.*") {

  tao_idlS.CONFIG += no_link
  tao_idlC.dependency_type = TYPE_C
  tao_idlS.input  = IDLFILES
  tao_idlS.output = ${QMAKE_FILE_BASE}S.cpp
  tao_idlS.clean  = ${QMAKE_FILE_BASE}S.cpp ${QMAKE_FILE_BASE}S.h ${QMAKE_FILE_BASE}S.inl
  silent::tao_idlS.commands = @echo $${TAO_IDL} $${IDL_FLAGS} ${QMAKE_FILE_IN}
  tao_idlS.variable_out = SOURCES
  tao_idlS.name   = TAO_IDL_S ${QMAKE_FILE_IN}
  QMAKE_EXTRA_COMPILERS += tao_idlS

} else {

  for(idl, IDLFILES): SOURCES += $$replace( idl, ".idl", "S.cpp" )
  for(idl, IDLFILES): SOURCES += $$replace( idl, ".idl", "C.cpp" )

}

