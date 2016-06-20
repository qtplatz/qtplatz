# qtplatz.cmake

#list( APPEND CMAKE_MODULE_PATH ${QTPLATZ_SOURCE_DIR}/cmake/Modules )
list( APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/Modules" )

message( STATUS "### CMAKE_CURRENT_LIST_DIR: " "${CMAKE_CURRENT_LIST_DIR}" )

find_package( arch )

#####################
# boost setup
#
if( WIN32 )

  find_path( _boost NAMES boost HINTS
    # "C:/Boost/include/boost-1_60"   # V14 
    "C:/Boost/include/boost-1_59"   # V13 
    "C:/Boost/include/boost-1_58"   # V12
    "C:/Boost/include/boost-1_57" )

  set( BOOST_ROOT ${_boost} )
  set( Boost_INCLUDE_DIR ${_boost} )
  set( Boost_USE_STATIC_LIBS ON )

  if ( RTC_ARCH_X64 )
    set( Boost_LIBRARY_DIR "C:/Boost/x86_64/lib" )
  else()
    set( Boost_LIBRARY_DIR "C:/Boost/lib" )    
  endif()

else()

  find_path( _boost NAMES include/boost HINTS
    "/usr/local"
    # "/usr/local/boost-1_60"        # V14
    "/usr/local/boost-1_59"        # V13
    "/usr/local/boost-1_58"        # V12
    "/usr/local/boost-1_57" )

  if ( _boost )
    set(Boost_INCLUDE_DIR "${_boost}/include")
    set(Boost_LIBRARY_DIR "${_boost}/lib")      
  endif()

endif()

#####################
# Qt5 setup
#

if ( NOT CMAKE_CROSSCOMPILING AND NOT QTPLATZ_CORELIB_ONLY )

  if ( WIN32 )
    if((MSVC_VERSION GREATER 1900) OR (MSVC_VERSION EQUAL 1900))
      set( CMAKE_PREFIX_PATH "C:/Qt/5.7/msvc2015_64" "C:/Qt/5.6/msvc2015_64" )
      find_package( Qt5 5.6 OPTIONAL_COMPONENTS Core QUIET )
    else()
      set( Qt5_DIR $ENV{QTDIR}/lib/cmake/Qt5 )
      find_package( Qt5 5.5 OPTIONAL_COMPONENTS Core QUIET )
    endif()
  else()
    # Qt5 location will be determined by PATH (qmake) on Linux & Apple are 
    find_package( Qt5 OPTIONAL_COMPONENTS Core QUIET ) #PATHS "/opt/Qt5.7.0" /opt/Qt/5.7 )
  endif()

  if ( Qt5_FOUND )
    get_filename_component( QTDIR "${Qt5_DIR}/../../.." ABSOLUTE ) # Qt5_DIR = ${QTDIR}/lib/cmake/Qt5
    
    find_program( QMAKE NAMES qmake HINTS "${QTDIR}/bin" "$ENV{QTDIR}" )
    message( STATUS "### QMAKE = " ${QMAKE} )
    if ( NOT QMAKE )
      message( FATAL_ERROR "qmake command not found" )
    endif()
    
    find_program( XMLPATTERNS NAMES xmlpatterns "${QTDIR}/bin" )
    message( STATUS "### XMLPATTERNS: " ${XMLPATTERNS} )
    if ( NOT XMLPATTERNS )
      message( FATAL_ERROR "xmlpatterns command not found" )
    endif()
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

  if ( ${CMAKE_SYSTEM_NAME} MATCHES Linux )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
  endif()

  if ( ${CMAKE_BUILD_TYPE} MATCHES DEBUG )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -DDEBUG")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -DDEBUG")    
  endif()

  if ( APPLE )
    add_definitions( "-Wno-unused-local-typedefs" )
  endif()

  add_definitions( "-Wno-deprecated-register" )

endif()

