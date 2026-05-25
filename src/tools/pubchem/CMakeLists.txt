cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0042 NEW )

project( pubchem )

if ( WIN32 )
  # OpenSSL default dir when build/install from source
  set ( OPENSSL_ROOT_DIR "C:/Program Files/OpenSSL" )
  set ( OPENSSL_USE_STATIC_LIBS TRUE )
endif()

find_package( OpenSSL )
if ( NOT OPENSSL_FOUND )
  return()
endif()

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

add_executable( pubchem
  main.cpp
)

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adportable
  ${Boost_LIBRARIES}
  certify::sshlib
  OpenSSL::SSL
  OpenSSL::Crypto
  pug
)

runtime_install_path( _runtime_destination ${PROJECT_NAME} )
library_install_path( _library_destination ${PROJECT_NAME} )
archive_install_path( _archive_destination ${PROJECT_NAME} )


install( TARGETS pubchem RUNTIME DESTINATION bin COMPONENT applications )
