cmake_minimum_required (VERSION 2.8.9)
cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0042 NEW )

project( tests )

#add_subdirectory( filename_splitter )
add_subdirectory( formula_parser )
add_subdirectory( test_ptree )
add_subdirectory( test_serializer )
#add_subdirectory( isotope )
add_subdirectory( test_adfs )
