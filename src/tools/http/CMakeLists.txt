#
# Copyright (c) 2016-2017 Vinnie Falco (vinnie dot falco at gmail dot com)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Official repository: https://github.com/boostorg/beast
#

project( http )

#add_library( sshlib INTERFACE )
#add_library( certify::sshlib ALIAS sshlib )

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

add_executable ( ${PROJECT_NAME}
  http_client_async_ssl.cpp
)

target_link_libraries( ${PROJECT_NAME}
  adportable
  ${Boost_LIBRARIES}
  certify::sshlib
  OpenSSL::SSL
  OpenSSL::Crypto
)
