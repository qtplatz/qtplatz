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

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT modules  
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )
