cmake_minimum_required (VERSION 2.8.9)
cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0042 NEW )

project( formula_parser )

set( Boost_USE_STATIC_LIBS ON )
set( BOOST_VERSION boost-1_57 )
if( WIN32 )
  set( BOOST_ROOT "C:/Boost/include/${BOOST_VERSION}" )
  set( Boost_LIBRARY_DIR "C:/Boost/x86_64/lib" )
else()
  set(Boost_INCLUDE_DIR "/usr/local/${BOOST_VERSION}/include")
  set(Boost_LIBRARY_DIR "/usr/local/${BOOST_VERSION}/lib")
endif()

find_package( Boost REQUIRED )

include_directories(
  ${Boost_INCLUDE_DIRS}
  ${QTPLATZ_SOURCE_DIR}/src/libs
  )

add_executable( formula_parser  formula_parser.cpp )
