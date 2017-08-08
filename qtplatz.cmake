# qtplatz.cmake

list( APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/Modules" )

message( STATUS "### CMAKE_CURRENT_LIST_DIR: " "${CMAKE_CURRENT_LIST_DIR}" )

find_package( arch )

#####################
# boost setup
#
set( Boost_NO_SYSTEM_PATHS ON )
set( Boost_ADDITIONAL_VERSIONS
  "1.63.0" "1.63"
  "1.62.0" "1.62" )

if( WIN32 )

  find_path( _boost NAMES boost HINTS
    "C:/Boost/include/boost-1_63"   # V14 
    "C:/Boost/include/boost-1_62"   # V14 
    "C:/Boost/include/boost-1_59"   # V13 
    "C:/Boost/include/boost-1_58"   # V12
    "C:/Boost/include/boost-1_57" )

  set( BOOST_ROOT ${_boost} )
  set( BOOST_INCLUDEDIR ${_boost} )
  if ( RTC_ARCH_X64 )
    set( BOOST_LIBRARYDIR "C:/Boost/x86_64/lib" )
  else()
    set( BOOST_LIBRARYDIR "C:/Boost/lib" )    
  endif()
  set( Boost_USE_STATIC_LIBS ON )

else()

  find_path( _boost NAMES include/boost HINTS
    "/usr/local/boost-1_63"        # V14 'libs/serialization/src/basic_archive.cpp library_version_type(14)
    "/usr/local/boost-1_62"        # V14
    "/usr/local/boost-1_59"        # V13
    "/usr/local/boost-1_58"        # V12
    "/usr/local/boost-1_57"
    "/usr/local"
    )

  if ( _boost )
    set( BOOST_ROOT ${_boost} )
    #set( BOOST_INCLUDEDIR ${_boost}/include )    
    #set( Boost_INCLUDE_DIR "${_boost}/include")
    #set( Boost_LIBRARY_DIR "${_boost}/lib")      
  endif()

endif()

#####################
# Qt5 setup
#

if ( WITH_QT5 )

  find_program( QMAKE NAMES qmake HINTS "${QTDIR}/bin" "$ENV{QTDIR}/bin" )

  if ( QMAKE ) 
    execute_process( COMMAND ${QMAKE} -query QT_INSTALL_PREFIX OUTPUT_VARIABLE __prefix )
    string( REGEX REPLACE "\n$" "" __prefix ${__prefix} )
    list( APPEND CMAKE_PREFIX_PATH "${__prefix}/lib/cmake" )
    set( QTDIR ${QT_INSTALL_PREFIX} )
  endif()

  find_package( Qt5 OPTIONAL_COMPONENTS Core QUIET )
  message( STATUS "### QMAKE = " ${QMAKE} )
  message( STATUS "### Qt5 = " ${Qt5} )
    
  if ( Qt5_FOUND )
    get_filename_component( QTDIR "${Qt5_DIR}/../../.." ABSOLUTE ) # Qt5_DIR = ${QTDIR}/lib/cmake/Qt5
    find_program( XMLPATTERNS NAMES xmlpatterns HINTS "${QTDIR}/bin" )
    message( STATUS "### XMLPATTERNS: " ${XMLPATTERNS} )
    if ( NOT XMLPATTERNS )
      message( FATAL_ERROR "xmlpatterns command not found" )
    endif()
  else()
    message( STATUS "# Qt5_DIR = ${Qt5_DIR}" )
    message( STATUS "# Disable QT5" )
    set( WITH_QT5 NO )
  endif()

  if ( QMAKE )
    execute_process( COMMAND ${QMAKE} -query QT_INSTALL_PREFIX
      OUTPUT_VARIABLE QTDIR ERROR_VARIABLE qterr OUTPUT_STRIP_TRAILING_WHITESPACE )

    execute_process( COMMAND ${QMAKE} -query QT_INSTALL_PLUGINS
      OUTPUT_VARIABLE QT_INSTALL_PLUGINS ERROR_VARIABLE qterr OUTPUT_STRIP_TRAILING_WHITESPACE )
    execute_process( COMMAND ${QMAKE} -query QT_INSTALL_LIBEXECS
      OUTPUT_VARIABLE QT_INSTALL_LIBEXECS ERROR_VARIABLE qterr OUTPUT_STRIP_TRAILING_WHITESPACE )
  endif()

endif()

#####################
# Compiler setup
#
if (MSVC)

  add_definitions( "-DUNICODE" "-D_UNICODE" "-D_WIN32_WINNT=0x0601" "-D_SCL_SECURE_NO_WARNINGS" )
  message(STATUS "Using ${CMAKE_CXX_COMPILER}. C++11 support is native.")

else()

  include(CheckCXXCompilerFlag)
  CHECK_CXX_COMPILER_FLAG("-std=c++17" COMPILER_SUPPORTS_CXX17)
  CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX14)
  CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
  CHECK_CXX_COMPILER_FLAG("-std=c++03" COMPILER_SUPPORTS_CXX03)

  if(COMPILER_SUPPORTS_CXX17)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
  elseif(COMPILER_SUPPORTS_CXX14)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
  elseif(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
  else()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++1y support. Please use a different C++ compiler.")
  endif()

  if ( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,-z,defs")
  endif()

  if ( ${CMAKE_BUILD_TYPE} MATCHES DEBUG )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -DDEBUG")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -DDEBUG")    
  endif()

endif()  

if ( MSVC )
  add_definitions( -wd4251 -wd4244 -wd4005 -wd4275 -wd4267 -wd4996 -wd4348 )
endif()

if ( APPLE )
  add_definitions( "-Wno-unused-local-typedefs" )
  add_definitions( -Wno-deprecated-declarations )
endif()

if ( CMAKE_COMPILER_IS_GNUCC )
  add_definitions( "-Wno-deprecated-declarations" )
endif()

add_library( QTC::Core SHARED IMPORTED )
add_library( QTC::ExtensionSystem SHARED IMPORTED )
add_library( QTC::Utils SHARED IMPORTED )
if (WIN32)
  set_target_properties( QTC::Core PROPERTIES
    IMPORTED_IMPLIB     ${QTPLATZ_ARCHIVE_OUTPUT_DIRECTORY}/Release/Core.lib
    IMPORTED_IMPLIB_DEBUG ${QTPLATZ_ARCHIVE_OUTPUT_DIRECTORY}/Debug/Cored.lib )

  set_target_properties( QTC::ExtensionSystem PROPERTIES
    IMPORTED_IMPLIB     ${QTPLATZ_ARCHIVE_OUTPUT_DIRECTORY}/Release/extensionsystem.lib
    IMPORTED_IMPLIB_DEBUG ${QTPLATZ_ARCHIVE_OUTPUT_DIRECTORY}/Debug/extensionsystemd.lib )

  set_target_properties( QTC::Utils PROPERTIES
    IMPORTED_IMPLIB     ${QTPLATZ_ARCHIVE_OUTPUT_DIRECTORY}/Release/Utils.lib
    IMPORTED_IMPLIB_DEBUG ${QTPLATZ_ARCHIVE_OUTPUT_DIRECTORY}/Debug/Utilsd.lib )
elseif ( APPLE )
  set_target_properties( QTC::Core PROPERTIES
    IMPORTED_LOCATION       ${QTPLATZ_PLUGIN_DIRECTORY}/QtProject/libCore.dylib
    IMPORTED_LOCATION_DEBUG ${QTPLATZ_PLUGIN_DIRECTORY}/QtProject/libCore_debug.dylib )

  set_target_properties( QTC::ExtensionSystem PROPERTIES
    IMPORTED_LOCATION       ${QTPLATZ_LIBRARY_OUTPUT_DIRECTORY}/libextensionsystem.dylib
    IMPORTED_LOCATION_DEBUG ${QTPLATZ_LIBRARY_OUTPUT_DIRECTORY}/libextensionsystem_debug.dylib )

  set_target_properties( QTC::Utils PROPERTIES
    IMPORTED_LOCATION       ${QTPLATZ_LIBRARY_OUTPUT_DIRECTORY}/libUtils.dylib
    IMPORTED_LOCATION_DEBUG ${QTPLATZ_LIBRARY_OUTPUT_DIRECTORY}/libUtils_debug.dylib )
else()  #Linux
  set_target_properties( QTC::Core PROPERTIES
    IMPORTED_LOCATION     ${QTPLATZ_LIBRARY_OUTPUT_DIRECTORY}/plugins/QtProject/libCore.so )

  set_target_properties( QTC::ExtensionSystem PROPERTIES
    IMPORTED_LOCATION     ${QTPLATZ_LIBRARY_OUTPUT_DIRECTORY}/libextensionsystem.so )

  set_target_properties( QTC::Utils PROPERTIES
    IMPORTED_LOCATION     ${QTPLATZ_LIBRARY_OUTPUT_DIRECTORY}/libutils.so )
endif()
