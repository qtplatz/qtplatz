# qtplatz.cmake

set( BOOST_VERSION boost-1_58 )

list( APPEND CMAKE_MODULE_PATH ${QTPLATZ_SOURCE_DIR}/cmake/Modules )

find_package( arch )

#####################
# boost setup
#
if( WIN32 )
  
  set( BOOST_ROOT "C:/Boost/include/${BOOST_VERSION}" )
  set( Boost_INCLUDE_DIR ${BOOST_ROOT} )
  set( Boost_USE_STATIC_LIBS ON )

  if ( RTC_ARCH_X64 )
    
    set( Boost_LIBRARY_DIR "C:/Boost/x86_64/lib" )
    
  else()

    set( Boost_LIBRARY_DIR "C:/Boost/lib" )    
    
  endif()

else()

  find_path( _boost NAMES include/boost PATHS "/usr/local" "/usr/local/${BOOST_VERSION}" )
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
    find_program( QMAKE NAMES qmake
      HINTS
      $ENV{QTDIR} "C:/Qt/5.5/msvc2013_64/bin" "C:/Qt/5.4/msvc2013_64/bin" )
  else()
    find_program( QMAKE NAMES qmake
      HINTS
      $ENV{QTDIR} "/opt/Qt/5.5/gcc_64/bin"
      )
  endif()

  if ( QMAKE )
    execute_process( COMMAND ${QMAKE} -query QT_INSTALL_PREFIX
      OUTPUT_VARIABLE QTDIR ERROR_VARIABLE qterr OUTPUT_STRIP_TRAILING_WHITESPACE )
    execute_process( COMMAND ${QMAKE} -query QT_INSTALL_PLUGINS
      OUTPUT_VARIABLE QT_INSTALL_PLUGINS ERROR_VARIABLE qterr OUTPUT_STRIP_TRAILING_WHITESPACE )
    execute_process( COMMAND ${QMAKE} -query QT_INSTALL_LIBEXECS
      OUTPUT_VARIABLE QT_INSTALL_LIBEXECS ERROR_VARIABLE qterr OUTPUT_STRIP_TRAILING_WHITESPACE )
    find_program( XMLPATTERNS NAMES xmlpatterns PATHS ${QT_INSTALL_LIBEXECS} )
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
  CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
  CHECK_CXX_COMPILER_FLAG("-std=c++03" COMPILER_SUPPORTS_CXX03)

  if(COMPILER_SUPPORTS_CXX11)
    if ( RTC_ARCH_ARM )
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    else()
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    endif()
  elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
  else()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
  endif()

  if ( ${CMAKE_SYSTEM_NAME} MATCHES Linux )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
  endif()

  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wdeprecated-register")

endif()

