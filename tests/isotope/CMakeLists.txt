cmake_minimum_required (VERSION 2.8.9)
cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0042 NEW )

project( isotope )

set( QTPLATZ_SORUCE_DIR ${CMAKE_SOURCE_DIR}/../.. )

find_path( qtplatz_DIR NAMES "qtplatz-config.cmake"  HINTS ../../build ../../release_build )

include ( ${qtplatz_DIR}/qtplatz-config.cmake )

find_package( Boost REQUIRED COMPONENTS system )


include_directories(
  ${Boost_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/../../src/libs
  )

add_definitions(-DADCONTROLS_LIBRARY) # fake for direct linking adcontrols.dll soruce code

add_executable( ${PROJECT_NAME}
  main.cpp
  ${QTPLATZ_SORUCE_DIR}/src/libs/adcontrols/chemicalformula.cpp
  ${QTPLATZ_SORUCE_DIR}/src/libs/adcontrols/isotopecluster.cpp
  ${QTPLATZ_SORUCE_DIR}/src/libs/adcontrols/tableofelement.cpp
  ${QTPLATZ_SORUCE_DIR}/src/libs/adcontrols/molecule.cpp
  ${QTPLATZ_SORUCE_DIR}/src/libs/adcontrols/ctable.cpp
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC ${Boost_LIBRARIES} adportable )
