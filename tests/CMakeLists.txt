cmake_minimum_required (VERSION 2.8.9)
cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0042 NEW )

project( tests )

find_package( Boost 1.62 REQUIRED COMPONENTS
  date_time
  filesystem
  iostreams
  locale
  log
  program_options
  regex
  serialization
  system
  thread
  timer
  wserialization
  unit_test_framework    
  )

if ( Boost_USE_STATIC_LIBS)
  add_definitions( -DBoost_USE_STATIC_LIBS=1 )
endif()

#add_subdirectory( filename_splitter )
add_subdirectory( formula_parser )
add_subdirectory( test_ptree )
add_subdirectory( test_serializer )
#add_subdirectory( isotope )
add_subdirectory( test_adfs )

if ( NOT CMAKE_CROSSCOMPILING )
  add_subdirectory( test_dataprocessor )
endif()
