cmake_minimum_required (VERSION 2.8.9)
cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0042 NEW )

project( tests )

if ( Boost_USE_STATIC_LIBS)
  add_definitions( -DBoost_USE_STATIC_LIBS=1 )
endif()

find_package( Boost 1.62 REQUIRED COMPONENTS
  unit_test_framework    
  )

#add_subdirectory( filename_splitter )
add_subdirectory( formula_parser )
add_subdirectory( test_ptree )
add_subdirectory( test_serializer )
#add_subdirectory( isotope )
add_subdirectory( test_adfs )
add_subdirectory( test_json )

if ( NOT CMAKE_CROSSCOMPILING )
  add_subdirectory( test_dataprocessor )
endif()
