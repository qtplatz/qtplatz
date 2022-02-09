cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0042 NEW )

project( csv_parser )

if ( NOT Boost_FOUND ) # -- standalone
  cmake_minimum_required(VERSION 3.22)
  set ( CMAKE_EXPORT_COMPILE_COMMANDS ON )

  if ( WIN32 )
    list( APPEND CMAKE_MODULE_PATH "C:/qtplatz"    "C:/opt/Eigen3/share/eigen3/cmake/")
    list( APPEND CMAKE_PREFIX_PATH "C:/opt/Eigen3" "C:/opt/RDKit/lib/cmakec/rdkit")
  elseif( APPLE )
    find_path( qtplatz_config_DIR NAMES "qtplatz-config.cmake"
      HINTS "$ENV{HOME}/src/build-Darwin-i386/qtplatz.release" )
    list( APPEND CMAKE_PREFIX_PATH ${qtplatz_config_DIR} )
  else()
    list( APPEND CMAKE_MODULE_PATH "/opt/qtplatz")
  endif()

  set( Boost_NO_WARN_NEW_VERSIONS ON )
  find_package( qtplatz REQUIRED )
  find_package( rdkit REQUIRED )
  find_package( Boost 1.75 REQUIRED COMPONENTS
    filesystem
    json
    program_options
    system
    )

  set(CMAKE_CXX_STANDARD 17)

endif()

include_directories(
  ${Boost_INCLUDE_DIRS}
  ${QTPLATZ_SOURCE_DIR}/src/libs
  )

add_executable( ${PROJECT_NAME}  main.cpp )

add_executable( ${PROJECT_NAME}2  main2.cpp )
