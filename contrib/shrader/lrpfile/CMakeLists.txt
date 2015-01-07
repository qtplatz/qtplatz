
add_library( lrpfile
  lrpfile.cpp
  lrpfile.hpp
  lrpheader.cpp
  lrpheader.hpp
  lrphead2.cpp
  lrphead2.hpp
  lrphead3.cpp
  lrphead3.hpp
  instsetup.cpp
  instsetup.hpp
  lrpcalib.cpp
  lrpcalib.hpp
  simions.cpp
  simions.hpp
  lrptic.cpp
  lrptic.hpp
  msdata.cpp
  msdata.hpp
  )

target_include_directories( lrpfile PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )
