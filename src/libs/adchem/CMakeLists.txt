
add_definitions(-DADCHEM_LIBRARY)

add_library( adchem
  adchem.cpp
  sdfile.cpp
  molfile.cpp
  drawing.cpp
  molecule.cpp
)

#LIBS += -lFileParsers \
#        -lGraphMol \
#        -lSmilesParse \
#        -lRDGeneral \
#        -lRDGeometryLib \
#        -lSubstructMatch \
#        -lDepictor \
#        -lDescriptors
#!win32 {
#  LIBS += -lboost_system
#}
#LIBS += -l$$qtLibraryTarget( adportable )

