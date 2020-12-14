# qtplatz.cmake

find_package( arch )

#####################
# boost setup
#
set( Boost_NO_SYSTEM_PATHS ON )

if ( WIN32 )

  # See 'libs/serialization/src/basic_archive.cpp library_version_type
  find_path( _boost NAMES boost HINTS
    "C:/Boost/include/boost-1_75"   # V18
    "C:/Boost/include/boost-1_73"   # V18
    "C:/Boost/include/boost-1_69"   # V17
    "C:/Boost/include/boost-1_67"   # V16
    "C:/Boost/include/boost-1_62"   # V14
    )

  set( BOOST_ROOT ${_boost} )
  set( BOOST_INCLUDEDIR ${_boost} )
  set( BOOST_LIBRARYDIR "C:/Boost/lib" )

  # add_definitions( -DBOOST_ALL_NO_LIB ) # disable auto linking

  # On windows, boost::archive templates are not possible to implment across shared object boundary
  set( Boost_USE_STATIC_LIBS ON )

  if ( Boost_USE_STATIC_LIBS )
    add_definitions(
      -DBOOST_LOG_DYN_LINK
      -DBOOST_ATOMIC_DYN_LINK
      -DBOOST_BZIP2_DYN_LINK
      -DBOOST_CHRONO_DYN_LINK
      -DBOOST_RANDOM_DYN_LINK
      -DBOOST_SYSTEM_DYN_LINK
      -DBOOST_TIMER_DYN_LINK
      )
  else()
    add_definitions( -DBOOST_ALL_DYN_LINK )
    add_definitions( -wd4141 ) # dllexport more than once
  endif()

else()
  ## Boost setup for mac/linux
  set( Boost_USE_STATIC_LIBS OFF )
  set( Boost_NO_SYSTEM_PATHS ON )

  find_path( _boost NAMES include/boost HINTS
    "/usr/local/boost-1_75"        # V18 <-- 'libs/serialization/src/basic_archive.cpp library_version_type(18 )
    "/usr/local/boost-1_73"        # V18 <-- 'libs/serialization/src/basic_archive.cpp library_version_type(18 )
    "/usr/local/boost-1_69"        # V17 <-- 'libs/serialization/src/basic_archive.cpp library_version_type(17)
    "/usr/local/boost-1_67"        # V16 <-- 'libs/serialization/src/basic_archive.cpp library_version_type(16)
    "/usr/local/boost-1_62"        # V14 <-- qtplatz acquisition 3.11.0 (debian9 default)
    "/usr/local/boost-1_59"        # V13
    "/usr/local/boost-1_58"        # V12
    "/usr/local/boost-1_57"
    "/usr/local"
    )

  if ( _boost )
    set( BOOST_ROOT ${_boost} )
  endif()
endif()

#####################
# Qt5 setup
#

if ( WITH_QT5 )

  set ( __qt5_versions
    "5.15.2" "5.15.1" "5.15.0"
    "5.14.2" "5.14.1"
    "5.12.7" "5.12.6"
    "5.12.5" "5.12.4" "5.12.3" "5.12.2" "5.12.1" "5.12.0" )

  if ( WIN32 )
    foreach( v ${__qt5_versions} )
      list ( APPEND __qmake_hints "C:/Qt/${v}/msvc2019_64/bin" )
      list ( APPEND __qmake_hints "C:/Qt/${v}/msvc2017_64/bin" )
    endforeach()
  elseif( APPLE )
    foreach( v ${__qt5_versions} )
      list ( APPEND __qmake_hints "$ENV{HOME}/Qt/${v}/clang_64/bin" )
      list ( APPEND __qmake_hints "/opt/Qt/${v}/clang_64/bin" )
    endforeach()
  else()
    if ( CMAKE_CROSSCOMPILING )
      foreach( v ${__qt5_versions} )
        list ( APPEND __qmake_hints "/usr/local/arm-linux-gnueabihf/opt/Qt/${v}/bin" )
      endforeach()
    else()
      foreach( v ${__qt5_versions} )
        list ( APPEND __qmake_hints "/opt/Qt/${v}/gcc_64/bin" )
      endforeach()
    endif()
  endif()

  find_program( QMAKE NAMES qmake HINTS ${__qmake_hints} )

  if ( QMAKE )
    execute_process( COMMAND ${QMAKE} -query QT_INSTALL_PREFIX OUTPUT_VARIABLE __prefix )
    string( REGEX REPLACE "\n$" "" __prefix ${__prefix} )
    list( APPEND CMAKE_PREFIX_PATH "${__prefix}/lib/cmake" )
    set( QTDIR ${__prefix} )
  else()
    message( "=============================================================" )
    message( "====== No QMAKE FOUND =======================================" )
    message( "=============================================================" )
  endif()

  find_package( Qt5 OPTIONAL_COMPONENTS Core QUIET )

  if ( Qt5_FOUND )
    find_package( Qt5 CONFIG REQUIRED PrintSupport Svg Core Widgets Gui )
    get_filename_component( QTDIR "${Qt5_DIR}/../../.." ABSOLUTE ) # Qt5_DIR = ${QTDIR}/lib/cmake/Qt5
    find_program( XMLPATTERNS NAMES xmlpatterns HINTS "${QTDIR}/bin" )
    # message( STATUS "### XMLPATTERNS: " ${XMLPATTERNS} )
    if ( NOT XMLPATTERNS )
      message( FATAL_ERROR "xmlpatterns command not found" )
    endif()
  else()
    message( STATUS "# Qt5_DIR = ${Qt5_DIR}" )
    message( STATUS "# Disable QT5" )
    #set( WITH_QT5 NO )
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
  set( CMAKE_CXX_STANDARD 17 )
  #message(STATUS "Using ${CMAKE_CXX_COMPILER}. C++11 support is native.")

else()

  include(CheckCXXCompilerFlag)
  CHECK_CXX_COMPILER_FLAG("-std=c++17" COMPILER_SUPPORTS_CXX17)
  CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX14)
  CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
  CHECK_CXX_COMPILER_FLAG("-std=c++03" COMPILER_SUPPORTS_CXX03)

  if(COMPILER_SUPPORTS_CXX17)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
    set( CMAKE_CXX_STANDARD 17 )
  elseif(COMPILER_SUPPORTS_CXX14)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
    set( CMAKE_CXX_STANDARD 14 )
  elseif(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    set( CMAKE_CXX_STANDARD 11 )
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
remove_definitions( "-DBOOST_NO_AUTO_PTR" )

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
