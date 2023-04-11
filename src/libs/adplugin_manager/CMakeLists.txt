#
# CMakeLists for adplugin shared library
#
project( adplugin_manager )

cmake_policy(SET CMP0028 NEW)
cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0020 NEW)

#find_package( Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED )

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

add_definitions(-DADPLUGIN_LIBRARY)

add_library( ${PROJECT_NAME} SHARED
  manager.cpp
  manager.hpp
  loader.cpp
  loader.hpp
  )

include_directories( ${QtCore_INCLUDE_DIRS}  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adcontrols
  adportable
  xmlparser
  adlog
  adplugin
  ${Boost_LIBRARIES}
  ${CMAKE_DL_LIBS}
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
