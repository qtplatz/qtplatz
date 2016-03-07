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

if ( NOT CMAKE_CROSSCOMPILING )

  if ( WIN32 )
    if((MSVC_VERSION GREATER 1900) OR (MSVC_VERSION EQUAL 1900))
      find_program( QMAKE NAMES qmake HINTS $ENV{QTDIR} "C:/Qt/Qt5.6.0/5.6/msvc2015_64/bin" )      
    else()
      find_program( QMAKE NAMES qmake PATHS $ENV{QTDIR} "C:/Qt/5.5/msvc2013_64/bin" "C:/Qt/5.4/msvc2013_64/bin" )
    endif()
  else()
    find_program( QMAKE NAMES qmake PATHS $ENV{QTDIR} "/opt/Qt/5.5/gcc_64/bin" )
  endif()

  if ( QMAKE )
    execute_process( COMMAND ${QMAKE} -query QT_INSTALL_PREFIX
      OUTPUT_VARIABLE QTDIR ERROR_VARIABLE qterr OUTPUT_STRIP_TRAILING_WHITESPACE )

    message( STATUS "### QTDIR: " ${QTDIR} )
    
    execute_process( COMMAND ${QMAKE} -query QT_INSTALL_PLUGINS
      OUTPUT_VARIABLE QT_INSTALL_PLUGINS ERROR_VARIABLE qterr OUTPUT_STRIP_TRAILING_WHITESPACE )
    execute_process( COMMAND ${QMAKE} -query QT_INSTALL_LIBEXECS
      OUTPUT_VARIABLE QT_INSTALL_LIBEXECS ERROR_VARIABLE qterr OUTPUT_STRIP_TRAILING_WHITESPACE )
    #find_program( XMLPATTERNS NAMES xmlpatterns HINTS ${QT_INSTALL_PREFIX}/bin ${QT_INSTALL_LIBEXECS} )
    #find_program( XMLPATTERNS NAMES xmlpatterns PATHS "${QTDIR}/bin" )
    set( XMLPATTERNS "${QTDIR}/bin/xmlpatterns" )
    message( STATUS "### XMLPATTERNS: " ${XMLPATTERNS} )    
  endif()

  find_package( Qt5 OPTIONAL_COMPONENTS Core QUIET PATHS ${QTDIR} )  

endif()

#####################
# Compiler setup
#
if (MSVC)

  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_WIN32_WINNT=0x0601")
  add_definitions(-DUNICODE -D_UNICODE)
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

  if ( APPLE )
    add_definitions( "-Wno-unused-local-typedefs" )
  endif()

  add_definitions( "-Wno-deprecated-register" )

endif()

