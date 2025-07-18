cmake_minimum_required( VERSION 3.5 )
cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0020 NEW)
cmake_policy(SET CMP0028 NEW)

project( mzml )

# find_package( BZip2 REQUIRED )

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)


if ( OpenSSL_FOUND )
  include_directories( "${OPENSSL_INCLUDE_DIR}" )
endif()

include_directories(
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/adplugins/admzml
  )

add_executable( ${PROJECT_NAME}
  main.cpp
  xmlwalker.cpp xmlwalker.hpp
  get_full_xpath.cpp
  cvparamlist.cpp cvparamlist.hpp
  ${CMAKE_SOURCE_DIR}/src/adplugins/admzml/xmltojson.cpp ${CMAKE_SOURCE_DIR}/src/adplugins/admzml/xmltojson.hpp
  ${CMAKE_SOURCE_DIR}/src/adplugins/admzml/accession.cpp
  ${CMAKE_SOURCE_DIR}/src/adplugins/admzml/accession.hpp
  ${CMAKE_SOURCE_DIR}/src/adplugins/admzml/mzmlspectrum.cpp
  ${CMAKE_SOURCE_DIR}/src/adplugins/admzml/mzmlchromatogram.cpp
  ${CMAKE_SOURCE_DIR}/src/adplugins/admzml/mzmldatumbase.cpp
  ${CMAKE_SOURCE_DIR}/src/adplugins/admzml/binarydataarray.cpp
  ${CMAKE_SOURCE_DIR}/src/adplugins/admzml/scan_protocol.cpp
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adcontrols
  adfs
  adplugin_manager
  adprocessor
  adportable
  ${Boost_LIBRARIES}
  ${BZIP2_LIBRARIES}
  OpenSSL::Crypto  # for base64_decoder
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT libraries
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries
  )
