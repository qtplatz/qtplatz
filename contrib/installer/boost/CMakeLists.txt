cmake_minimum_required (VERSION 2.8.11)
cmake_policy( SET CMP0020 NEW )

project (boost-package)

find_package( Boost 1.57 REQUIRED )

if ( WIN32 )
  file( GLOB _libs "${Boost_LIBRARY_DIRS}/libboost_*.dll" )
elseif( APPLE )
  file( GLOB _libs "${Boost_LIBRARY_DIRS}/libboost_*.dylib" )  
else()
  file( GLOB _libs RELATIVE "${Boost_LIBRARY_DIRS}" "${Boost_LIBRARY_DIRS}/libboost_*.so*" )
endif()

if ( CMAKE_SYSTEM_NAME MATCHES Linux )

  set( CPACK_GENERATOR DEB )
  set( CPACK_DEBIAN_PACKAGE_MAINTAINER "T. Hondo <toshi.hondo@qtplatz.com>" )
  set( CPACK_DEBIAN_PACKAGE_VERSION "${VERSION}" )
  set( CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_BINARY_DIR}/postinst" )

  if ( CMAKE_CROSSCOMPILING )
    include(CheckSymbolExists)    
    check_symbol_exists( "__arm__" "" RTC_ARCH_ARM )
    if ( RTC_ARCH_ARM )
      set( CPACK_DEBIAN_PACKAGE_ARCHITECTURE armhf )
    endif()
  endif()

  set( CPACK_SET_DESTDIR ON )
  if ( "${CPACK_INSTALL_PREFIX}" STREQUAL "" )
    set( CPACK_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX} )
  endif()

  set( TARGET_LIBRARY_DIRECTORY ${CPACK_INSTALL_PREFIX}/lib )    

  # replace TARGET_LIBRARY_DIRECTORY with actual value at install-destination
  configure_file( ${CMAKE_SOURCE_DIR}/postinst.in ${CMAKE_BINARY_DIR}/postinst @ONLY )
  
endif()

message( "##### install prefix: " ${CMAKE_INSTALL_PREFIX} ";" ${CPACK_INSTALL_PREFIX} )
message( "##### boost runtime_directory: " ${TARGET_LIBRARY_DIRECTORY} )
message( "##### boost source directory: " ${Boost_LIBRARY_DIRS} )
message( "##### boost version: " ${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION} )

set( CPACK_PACKAGE_NAME "libboost-dev" )
set( CPACK_PACKAGE_VENDOR "ms-cheminfo.com" )
set( CPACK_PACKAGE_VERSION ${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION} )
set( CPACK_PACKAGE_VERSION_MAJOR ${Boost_MAJOR_VERSION} )
set( CPACK_PACKAGE_VERSION_MINOR ${Boost_MINOR_VERSION} )
set( CPACK_PACKAGE_VERSION_PATCH ${Boost_SUBMINOR_VERSION} )

include( CPackComponent )
cpack_add_component( runtime_libraries DISPLAY_NAME "Runtime libraries" DESCRIPTION "libraries" GROUP Runtime )
cpack_add_component( libraries         DISPLAY_NAME "Development libraries" DESCRIPTION "static and import libs" GROUP Development )
cpack_add_component( headers           DISPLAY_NAME "C++ Headers" DESCRIPTION "C++ header files" GROUP Development )

# Currently, runtime libraries only include into package
foreach( lib ${_libs} )
  install( FILES ${Boost_LIBRARY_DIRS}/${lib} DESTINATION lib COMPONENT runtime_libraries )
endforeach()

install( DIRECTORY ${Boost_INCLUDES_DIRS} DESTINATION . COMPONENT headers )

include( CPack )
