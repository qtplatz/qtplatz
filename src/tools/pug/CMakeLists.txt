cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0042 NEW )

project( pug )

#set (OPENSSL_ROOT_DIR "/usr/local/Cellar/libressl/3.4.2/include" )
#set (OPENSSL_ROOT_DIR "/usr/local/Cellar/libressl/3.4.2" )

find_package( OpenSSL )

if ( NOT OPENSSL_FOUND )
  return()
endif()

message( STATUS "######################## OpenSSL :               ${OPENSSL_FOUND}" )
message( STATUS "######################## OPENSSL LIBRARIES:      ${OPENSSL_LIBRARIES}" )
message( STATUS "######################## OPENSSL VERSION:        ${OPENSSL_VERSION}" )
message( STATUS "######################## OPENSSL APPLINK_SOURCE: ${OPENSSL_APPLINK_SOURCE}" )

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(
  "${Boost_INCLUDE_DIRS}"
  "${OPENSSL_INCLUDE_DIR}"
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
  ${OPENSSL_LIBRARIES}
  )
