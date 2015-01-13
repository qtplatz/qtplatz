cmake_policy( SET CMP0020 NEW )
project( adtextfile )

find_package( Qt5 CONFIG REQUIRED Core )

configure_file(
  "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.adplugin"
  "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${PROJECT_NAME}.adplugin"
  )

include_directories( ${Qt5Core_INCLUDE_DIRS}  )

add_definitions(-DADTXTFACTORY_LIBRARY)

add_library( adtextfile MODULE 
  adtextfile.cpp
  datafile.cpp
  datafile_factory.cpp
  txtspectrum.cpp
  txtchromatogram.cpp
  adtextfile.hpp
  datafile.hpp
  datafile_factory.hpp
  txtspectrum.hpp
  txtchromatogram.hpp
  )

target_link_libraries( adtextfile LINK_PUBLIC
  ${Boost_LIBRARIES}
  adcontrols
  adlog
  adportable
  xmlparser
  portfolio
  adplugin
)


