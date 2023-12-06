cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0042 NEW )

project( pug )

find_package( OpenSSL )

if ( NOT OPENSSL_FOUND )
  return()
endif()

add_library( core INTERFACE )
add_library( certify::core ALIAS core )

if ( APPLE )
  target_link_libraries(core INTERFACE "-framework CoreFoundation" "-framework Security")
  set_target_properties(core PROPERTIES LINK_FLAGS "-Wl,-F/Library/Frameworks")
endif()
if ( MSVC )
  target_link_libraries(core INTERFACE Crypt32.lib)
endif()

message( STATUS "######################## OpenSSL :               ${OPENSSL_FOUND}" )
message( STATUS "######################## OPENSSL LIBRARIES:      ${OPENSSL_LIBRARIES}" )
message( STATUS "######################## OPENSSL VERSION:        ${OPENSSL_VERSION}" )
message( STATUS "######################## OPENSSL APPLINK_SOURCE: ${OPENSSL_APPLINK_SOURCE}" )

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(
  "${Boost_INCLUDE_DIRS}"
  "${OPENSSL_INCLUDE_DIR}"
  "${CMAKE_SOURCE_DIR}/contrib"
)

add_executable( ${PROJECT_NAME}
  main.cpp
  http_client_async.cpp
  http_client_async.hpp
  )

target_link_libraries( ${PROJECT_NAME}
  adportable
  adfs
  adui
  adlog
  qtwrapper
  xmlparser
  ${Boost_LIBRARIES}
  certify::core
  OpenSSL::SSL
  OpenSSL::Crypto
  )

install( TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin COMPONENT applications )
