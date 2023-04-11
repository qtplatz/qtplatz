project( portfolio )

cmake_policy(SET CMP0022 NEW)

add_definitions(-DPORTFOLIO_LIBRARY)

add_library( portfolio SHARED
  portfolio.cpp
  folder.cpp
  folium.cpp
  node.cpp
  portfolioimpl.cpp
  logging_hook.cpp
  portfolio.hpp
  portfolio_global.h
  folder.hpp
  folium.hpp
  portfolioimpl.hpp
  logging_hook.hpp
)

target_link_libraries( portfolio LINK_PUBLIC
  xmlparser
  adportable
  ${Boost_LIBRARIES}
  )

runtime_install_path( _runtime_destination ${PROJECT_NAME} )
library_install_path( _library_destination ${PROJECT_NAME} )
archive_install_path( _archive_destination ${PROJECT_NAME} )

install( TARGETS ${PROJECT_NAME}
  EXPORT ${QTPLATZ_TARGETS}
  RUNTIME DESTINATION ${_runtime_destination} COMPONENT applications
  LIBRARY DESTINATION ${_library_destination} COMPONENT runtime_libraries
  ARCHIVE DESTINATION ${_archive_destination} COMPONENT libraries
  )
