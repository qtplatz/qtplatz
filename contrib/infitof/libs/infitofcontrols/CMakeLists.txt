cmake_policy( SET CMP0028 NEW )
cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0022 NEW )

project( infitofcontrols )
set( PROVIDER MS-Cheminformatics )

add_definitions( -DINFITOFCONTROLS_LIBRARY )

include_directories(
  ${CMAKE_SOURCE_DIR}/contrib/multum/libs
  ${CMAKE_SOURCE_DIR}/contrib/infitof/include
  ${CMAKE_SOURCE_DIR}/src/libs
  ${CMAKE_SOURCE_DIR}
  )

if ( MSVC )
  add_definitions(-D_SCL_SECURE_NO_WARNINGS -wd4996 )
endif()

add_library( ${PROJECT_NAME} SHARED
  avgrmethod.cpp
  avgrmethod.hpp
  dghelper.cpp
  dghelper.hpp
  infitofcontrols_global.hpp
#  ionsourcemethod.hpp
#  ionsourcemethod.cpp
  method.hpp
  method.cpp
#  methodeditorbehavior.hpp
#  methodeditorbehavior.cpp
#  voltagemethod.cpp
#  voltagemethod.hpp
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adacquire
  adplugin
  adportable
  adcontrols
  admtcontrols
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
