
#cmake_policy(SET CMP0020 NEW)
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
  ${Boost_LIBRARIES}
  adportable  
  xmlparser )

#win32: LIBS += -lole32
#!win32: LIBS += -lboost_date_time -lboost_system 
