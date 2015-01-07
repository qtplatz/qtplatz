
#find_package( Qt5 CONFIG REQUIRED Core )

#include_directories( ${CMAKE_SOURCE_DIR}/../PI-E_871
#                     ${Qt5Core_INCLUDE_DIRS}
#					 )

add_library( shrader_lrpfile
  shrader_lrpfile.cpp
)

#target_include_directories( sitcp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )
