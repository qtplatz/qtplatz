cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0042 NEW )

project( pug )

add_definitions(-DPUG_LIBRARY)

if ( WIN32 )
  # OpenSSL default dir when build/install from source
  set ( OPENSSL_ROOT_DIR "C:/Program Files/OpenSSL" )
  set ( OPENSSL_USE_STATIC_LIBS TRUE )
endif()

find_package( OpenSSL )
if ( NOT OPENSSL_FOUND )
  return()
endif()

add_library( sshlib INTERFACE )
add_library( certify::sshlib ALIAS sshlib )

if ( APPLE )
  target_link_libraries(sshlib INTERFACE "-framework CoreFoundation" "-framework Security")
  set_target_properties(sshlib PROPERTIES LINK_FLAGS "-Wl,-F/Library/Frameworks")
endif()
if ( MSVC )
  target_link_libraries(sshlib INTERFACE Crypt32.lib)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(
  "${Boost_INCLUDE_DIRS}"
  "${OPENSSL_INCLUDE_DIR}"
  "${CMAKE_SOURCE_DIR}/contrib"
)

add_library( ${PROJECT_NAME} SHARED
  http_client_async.cpp
  http_client_async.hpp
)

add_executable( pubchem
  main.cpp
)

target_link_libraries( ${PROJECT_NAME}
  adportable
  ${Boost_LIBRARIES}
  certify::sshlib
  OpenSSL::SSL
  OpenSSL::Crypto
)

runtime_install_path( _runtime_destination ${PROJECT_NAME} )
library_install_path( _library_destination ${PROJECT_NAME} )
archive_install_path( _archive_destination ${PROJECT_NAME} )

install( TARGETS sshlib EXPORT ${QTPLATZ_TARGETS} )

install( TARGETS ${PROJECT_NAME}
  EXPORT ${QTPLATZ_TARGETS}
  RUNTIME DESTINATION ${_runtime_destination} COMPONENT applications
  LIBRARY DESTINATION ${_library_destination} COMPONENT runtime_libraries
  ARCHIVE DESTINATION ${_archive_destination} COMPONENT libraries
)

target_link_libraries( pubchem
  adportable
  ${PROJECT_NAME}
  ${Boost_LIBRARIES}
  certify::sshlib
  OpenSSL::SSL
  OpenSSL::Crypto
  pug
)

install( TARGETS pubchem RUNTIME DESTINATION bin COMPONENT applications )
