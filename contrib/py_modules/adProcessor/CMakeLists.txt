cmake_minimum_required ( VERSION 3.13 )

project ( adProcessor )

find_package( Boost 1.67 COMPONENTS python${Python3_VERSION_MAJOR}${Python3_VERSION_MINOR} REQUIRED )
if ( Boost_FOUND )
  include_directories( ${Boost_INCLUDE_DIRS} )
endif()

if ( Python3_FOUND )
  include_directories( ${Python3_INCLUDE_DIRS} )
endif()

add_library( ${PROJECT_NAME} MODULE
  adprocessor.cpp
  dataprocessor.cpp
  dataprocessor.hpp
  datareader.cpp
  datareader.hpp
  file.cpp
  file.hpp
  folder.cpp
  folder.hpp
  )

link_directories(
  ${QTPLATZ_BINARY_DIR}/lib/qtplatz  # <== qmake .lib output dir
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adcontrols
  adfs
  adplugin_manager
  adprocessor
  acqrscontrols
  adportable
  ${Python3_LIBRARIES}
  ${Boost_LIBRARIES}
  Qt5::Widgets
  Qt5::Core
  )

if ( WIN32 )
  set ( suffix ".dll" )
else()
  set ( suffix ".so" )
endif()

set_target_properties( ${PROJECT_NAME}
  PROPERTIES
  PREFIX ""
  SUFFIX ${suffix}
  LIBRARY_OUTPUT_DIRECTORY ${PYTHON_MODULE_DIR}
  LIBRARY_OUTPUT_DIRECTORY_DEBUG ${PYTHON_MODULE_DIR}
  LIBRARY_OUTPUT_DIRECTORY_RELEASE ${PYTHON_MODULE_DIR}
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION "${PYTHON_DIST_PACKAGES}/qtplatz" COMPONENT modules
  LIBRARY DESTINATION "${PYTHON_DIST_PACKAGES}/qtplatz" COMPONENT modules
  )
