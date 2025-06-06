cmake_minimum_required ( VERSION 3.18.2 )

project ( ws_adprocessor )

#find_package( Qt${QT_VERSION_MAJOR} CONFIG REQUIRED Core )

include_directories(
  ${WSTP_INCLUDE_DIRS}
  ${LIBRARYLINK_INCLUDE_DIRS}
  ${QtCore_INCLUDE_DIRS}
  )

link_directories( ${WSTP_LIBRARY_DIRS} )

link_directories(
  ${WSTP_LIBRARY_DIRS}
  ${QTPLATZ_BINARY_DIR}/lib/qtplatz  # <== qmake .lib output dir
  )

WSTP_ADD_TM(adprocessor.tm .cxx)

add_executable( ${PROJECT_NAME}
  adprocessor.cpp
  adprocessor.tm.cxx
  dataprocessor.cpp
  dataprocessor.hpp
  datareader.cpp
  datareader.hpp
  singleton.cpp
  singleton.cpp
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adcontrols
  adfs
  adplugin_manager
  adprocessor
  acqrscontrols
  adportable
  Qt::Core
  ${Boost_LIBRARIES}
  ${WSTP_LIBRARIES}
  )

add_library( wsadprocessor SHARED
  wsadprocessor.cpp
  dataprocessor.cpp
  dataprocessor.hpp
  datareader.cpp
  datareader.hpp
  singleton.cpp
  singleton.cpp
  )

target_link_libraries( wsadprocessor LINK_PUBLIC
  adcontrols
  adfs
  adplugin_manager
  adprocessor
  acqrscontrols
  adportable
  Qt::Core
  ${Boost_LIBRARIES}
  ${WSTP_LIBRARIES}
  )

set_target_properties( ${PROJECT_NAME}
  PROPERTIES
  SUFFIX ".bin"
  RUNTIME_OUTPUT_DIRECTORY ${WSTP_MODULE_DIR}
  RUNTIME_OUTPUT_DIRECTORY_DEBUG ${WSTP_MODULE_DIR}
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${WSTP_MODULE_DIR}
  LIBRARY_OUTPUT_DIRECTORY ${WSTP_MODULE_DIR}
  LIBRARY_OUTPUT_DIRECTORY_DEBUG ${WSTP_MODULE_DIR}
  LIBRARY_OUTPUT_DIRECTORY_RELEASE ${WSTP_MODULE_DIR}
  )

set_target_properties( wsadprocessor
  PROPERTIES
  PREFIX ""
  RUNTIME_OUTPUT_DIRECTORY ${WSTP_MODULE_DIR}
  RUNTIME_OUTPUT_DIRECTORY_DEBUG ${WSTP_MODULE_DIR}
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${WSTP_MODULE_DIR}
  LIBRARY_OUTPUT_DIRECTORY ${WSTP_MODULE_DIR}
  LIBRARY_OUTPUT_DIRECTORY_DEBUG ${WSTP_MODULE_DIR}
  LIBRARY_OUTPUT_DIRECTORY_RELEASE ${WSTP_MODULE_DIR}
  )

install( TARGETS ${PROJECT_NAME} wsadprocessor
  RUNTIME DESTINATION "${WSTP_DIST_PACKAGES}" COMPONENT modules
  LIBRARY DESTINATION "${WSTP_DIST_PACKAGES}" COMPONENT modules
  )
