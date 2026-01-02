cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0042 NEW )

project( tests )

set( CMAKE_CXX_STANDARD 20 )

#find_package( Boost 1.75 REQUIRED COMPONENTS
#  unit_test_framework
#  json
#  )

#add_subdirectory( filename_splitter )
#add_subdirectory( test_svg )
#add_subdirectory( formula_parser )
#add_subdirectory( csv_parser )
#add_subdirectory( test_peptide )
#add_subdirectory( test_ptree )
#add_subdirectory( test_serializer )
#add_subdirectory( isotope )
#add_subdirectory( test_formula )
#add_subdirectory( test_adfs )
add_subdirectory( test_date )
#add_subdirectory( test_json )
#add_subdirectory( test_pkarea )
#add_subdirectory( test_regex )
#add_subdirectory( test_numpy )
if ( APPLE )
#  add_subdirectory( test_pge2 )
endif()

if ( NOT CMAKE_CROSSCOMPILING )
#  add_subdirectory( test_dataprocessor )
endif()
