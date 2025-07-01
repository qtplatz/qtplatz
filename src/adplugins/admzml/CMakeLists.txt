cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0028 NEW )

project( admzml )

set( PROVIDER MS-Cheminformatics )
set( CMAKE_INCLUDE_CURRENT_DIR ON )

qtplatz_adplugin_output_dir( _adplugin_output_dir ${PROVIDER} )
qtplatz_adplugin_install_dir( _adplugin_install_dir ${PROVIDER} )

set( OUTPUT_DIR "${CMAKE_BINARY_DIR}/${_adplugin_output_dir}" )

include_directories(
  ${CMAKE_SOURCE_DIR}/src    # app_version.h
  ${Qt${QT_VERSION_MAJOR}Core_INCLUDE_DIRS}  # for QJsonDocument (debug)
  )

add_definitions(-DADTXTFACTORY_LIBRARY)

if ( WIN32 )
  set ( BASE64LIB ${CMAKE_SOURCE_DIR}/src/libs/adportable/base64.cpp )
endif()

# An issue need to be resolved
# several .cpp files are refered from adimport project

add_library( ${PROJECT_NAME} MODULE
  admzml.cpp
  accession.cpp accession.hpp
  binarydataarray.cpp binarydataarray.hpp
  datafile.cpp datafile.hpp
  datafile_factory.cpp datafile_factory.hpp
  datainterpreter.cpp datainterpreter.hpp
  datareader.cpp datareader.hpp
  datareader_ex.cpp datareader_ex.hpp
  datareader_factory.cpp datareader_factory.hpp
  export_to_adfs.cpp export_to_adfs.hpp
  mzml.cpp mzml.hpp
  mzmlchromatogram.cpp mzmlchromatogram.hpp
  mzmldatumbase.cpp mzmldatumbase.hpp
  mzmlreader.cpp mzmlreader.hpp
  mzmlspectrum.cpp mzmlspectrum.hpp
  mzmlwalker.cpp mzmlwalker.hpp
  scan_protocol.cpp scan_protocol.hpp
  serializer.cpp serializer.hpp
  xmltojson.cpp xmltojson.hpp
  ${BASE64LIB}
)

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adportable
  adcontrols
  adlog
  xmlparser
  portfolio
  adplugin
  adfs
  ${Boost_LIBRARIES}
  ${ZLIB_LIBRARIES}
  Threads::Threads
  Qt6::Core
)

set_target_properties( ${PROJECT_NAME}
  PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION "${_adplugin_install_dir}" COMPONENT plugins
  LIBRARY DESTINATION "${_adplugin_install_dir}" COMPONENT plugins
  ARCHIVE DESTINATION "lib/qtplatz" COMPONENT libraries
)

########
