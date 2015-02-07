project( portfolio )

#cmake_policy(SET CMP0020 NEW)
cmake_policy(SET CMP0022 NEW)

add_definitions(-DPORTFOLIO_LIBRARY)

find_package(Boost COMPONENTS date_time system )

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
  ${Boost_LIBRARIES}
  adportable  
  xmlparser )

set_target_properties( portfolio
  PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY ${QTPLATZ_BINARY_DIR}/lib/qtplatz )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT modules  
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )
