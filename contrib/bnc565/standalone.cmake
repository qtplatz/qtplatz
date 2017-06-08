
if ( NOT ${CMAKE_BUILD_TYPE} STREQUAL "" )
  string( TOLOWER ${CMAKE_BUILD_TYPE} build_type )
else()
  set( build_type "release" )
endif()

list( APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../cmake/Modules" )

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

#  if ( ${CMAKE_SYSTEM_NAME} MATCHES Linux )
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
#    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
#  endif()

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

if ( WIN32 )
  set( CMAKE_INSTALL_PREFIX "C:/QtPlatz" )
elseif( APPLE )
  set( CMAKE_INSTALL_PREFIX "/opt/qtplatz" )
else()
  set( CMAKE_INSTALL_PREFIX "/opt/qtplatz" )
endif()
