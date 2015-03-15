cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0028 NEW )

add_definitions(-DSHRADER_LRPFILE_LIBRARY)

add_library( shrader_lrpfile SHARED
  shrader_lrpfile.cpp
  datafile_factory.cpp
  datafile_factory.hpp
  datafile.cpp
  datafile.hpp
)

target_link_libraries( shrader_lrpfile LINK_PUBLIC
  ${Boost_LIBRARIES}
  lrpfile  
  adplugin
  adcontrols
  portfolio )


