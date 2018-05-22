######
## RDKit install on Windows
##
set ( RDKIT_SOURCE_DIR ${RDBASE} )
set ( RDKIT_BUILD_DIR ${BUILD_ROOT}/rdkit.build )
if ( NOT EXISTS ${RDKIT_BUILD_DIR} )
  file( MAKE_DIRECTORY ${RDKIT_BUILD_DIR} )
endif()

add_custom_command(
  OUTPUT ${RDKIT_SOURCE_DIR}
  COMMAND git clone https://github.com/rdkit/rdkit ${RDKIT_SOURCE_DIR}  
  )

add_custom_command(
  DEPENDS ${RDKIT_SOURCE_DIR}
  COMMAND git checkout -b Release_2018_03_1
  COMMAND ${CMAKE_COMMAND} -E touch "${RDKIT_RELEASE}.date"
  OUTPUT "${RDKIT_RELEASE}.date"
  WORKING_DIRECTORY ${RDKIT_SOURCE_DIR}
  )

add_custom_command(
  OUTPUT ${RDKIT_BUILD_DIR}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${RDKIT_BUILD_DIR}
  )

if ( $ENV{VisualStudioVersion} VERSION_LESS "15.0" )
  message( FATAL "Visual Studio Version $ENV{VisualStudioVersion} too old" )
else()
  set ( GENERATOR "Visual Studio 15 2017 Win64" )
endif()

find_package( Boost 1.57 REQUIRED COMPONENTS
  atomic
  bzip2
  chrono
  date_time
  filesystem
  iostreams
  locale
  program_options
  regex
  serialization
  system
  thread
  wserialization )

add_custom_target( rdkit
  DEPENDS ${RDKIT_SOURCE_DIR} ${RDKIT_BUILD_DIR} "${RDKIT_RELEASE}.date"
  COMMAND cmake
  -DBOOST_LIBRARYDIR="C:/Boost/lib"
  -DBOOST_ROOT=${BOOST_ROOT}
  -DBoost_USE_STATIC_LIBS=ON
  -DRDK_BUILD_INCHI_SUPPORT=ON
  -DRDK_BUILD_PYTHON_WRAPPERS=OFF
  -DRDK_BUILD_SWIG_JAVA_WRAPPER=OFF
  -DRDK_INSTALL_STATIC_LIBS=ON
  -DRDK_INSTALL_DYNAMIC_LIBS=OFF
  -DCMAKE_DEBUG_POSTFIX="d" -G ${GENERATOR} -DCMAKE_CXX_FLAGS="/MP" ${RDBASE} ${RDKIT_SOURCE_DIR}
  COMMAND cmake --build . --config Debug
  COMMAND cmake --build . --config Release
  COMMAND cmake --build . --target install
  WORKING_DIRECTORY ${RDKIT_BUILD_DIR}
  )

## end boost install.
######
