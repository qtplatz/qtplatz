cmake_minimum_required ( VERSION 3.13 )

project (world)

find_package( Boost 1.67 COMPONENTS python${Python3_VERSION_MAJOR}${Python3_VERSION_MINOR} REQUIRED )
if ( Boost_FOUND )
  include_directories( ${Boost_INCLUDE_DIRS} )
endif()

if ( Python3_FOUND )
  include_directories( ${Python3_INCLUDE_DIRS} )
endif()

add_library( ${PROJECT_NAME} MODULE
  world.cpp
  world.hpp
  py_world.cpp
)

target_link_libraries( ${PROJECT_NAME}
  ${Python3_LIBRARIES}
  ${Boost_LIBRARIES}
)

add_library( hello_ext MODULE
  hello.cpp
)

target_link_libraries( hello_ext
  ${Python3_LIBRARIES}
  ${Boost_LIBRARIES}
)

if ( WIN32 )
  set ( suffix ".pyd" )
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

set_target_properties( hello_ext
  PROPERTIES
  PREFIX ""
  SUFFIX ${suffix}
  LIBRARY_OUTPUT_DIRECTORY ${PYTHON_MODULE_DIR}
  LIBRARY_OUTPUT_DIRECTORY_DEBUG ${PYTHON_MODULE_DIR}
  LIBRARY_OUTPUT_DIRECTORY_RELEASE ${PYTHON_MODULE_DIR}
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION "${PYTHON_DIST_PACKAGES}/qtplatz" COMPONENT py_modules
  LIBRARY DESTINATION "${PYTHON_DIST_PACKAGES}/qtplatz" COMPONENT py_modules
  )
