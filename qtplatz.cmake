# qtplatz.cmake

find_package( arch )

#####################
# Qt5 setup
#
if ( WITH_QT5 )

  if ( QT_FOUND )
    find_program( QMAKE NAMES qmake )
  endif()

  if ( QMAKE )
    execute_process( COMMAND ${QMAKE} -query QT_INSTALL_PREFIX
      OUTPUT_VARIABLE QTDIR ERROR_VARIABLE qterr OUTPUT_STRIP_TRAILING_WHITESPACE )
    execute_process( COMMAND ${QMAKE} -query QT_INSTALL_PLUGINS
      OUTPUT_VARIABLE QT_INSTALL_PLUGINS ERROR_VARIABLE qterr OUTPUT_STRIP_TRAILING_WHITESPACE )
    execute_process( COMMAND ${QMAKE} -query QT_INSTALL_LIBEXECS
      OUTPUT_VARIABLE QT_INSTALL_LIBEXECS ERROR_VARIABLE qterr OUTPUT_STRIP_TRAILING_WHITESPACE )
  else()
    message( STATUS "=============================================================" )
    message( STATUS "====== No QMAKE FOUND =======================================" )
    message( STATUS "=============================================================" )
  endif()

  if ( QT_FOUND )
    find_program( XMLPATTERNS NAMES xmlpatterns HINTS "${QTDIR}/bin" "/opt/Qt/5.15.2/bin" )
    if ( NOT XMLPATTERNS )
      message( FATAL_ERROR "xmlpatterns command not found" )
    endif()
  endif()

endif()

#####################
# Eigen3
#
if ( MSVC )
  find_path( __eigen3_include_path signature_of_eigen3_matrix_library
    HINTS "C:/opt/Eigen3/include" "C:/Eigen3/include" PATH_SUFFIXES "eigen3" "eigen" )
  if ( __eigen3_include_path )
    get_filename_component( __eigen3_dir "${__eigen3_include_path}/../.." ABSOLUTE )
    list ( APPEND CMAKE_PREFIX_PATH ${__eigen3_dir} )
  endif()
endif()


#####################
# Compiler setup
#
if (MSVC)

  add_definitions( "-DUNICODE" "-D_UNICODE" "-D_WIN32_WINNT=0x0601" "-D_SCL_SECURE_NO_WARNINGS" )
  add_definitions(  -wd4251 -wd4244 -wd4819 )
  add_definitions(  -wd4141 ) # dllexport more than once
  add_definitions(  -wd4217 ) # conditional expression is constant
  add_definitions(  -wd4003 ) # insufficient arguments for function style macro call
  add_definitions(  -wd6011 ) # null pointer evaluated (caused for Q_CHECK_PTR)
  add_definitions( -wd26451 ) # integer arithmetics overflow
  add_definitions( -wd26495 ) # uninitialized member variable
  add_definitions( -wd26498 ) # constexpr
  set( CMAKE_CXX_STANDARD 20 )
  set( COMPILER_SUPPORTS_CXX20 TRUE )

else()

  include(CheckCXXCompilerFlag)
  CHECK_CXX_COMPILER_FLAG("-std=c++23" COMPILER_SUPPORTS_CXX23)
  CHECK_CXX_COMPILER_FLAG("-std=c++20" COMPILER_SUPPORTS_CXX20)
  CHECK_CXX_COMPILER_FLAG("-std=c++17" COMPILER_SUPPORTS_CXX17)
  CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX14)
  CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
  CHECK_CXX_COMPILER_FLAG("-std=c++03" COMPILER_SUPPORTS_CXX03)

  if(COMPILER_SUPPORTS_CXX23)
    set( CMAKE_CXX_STANDARD 23 )
  elseif(COMPILER_SUPPORTS_CXX20)
    set( CMAKE_CXX_STANDARD 20 )
  elseif(COMPILER_SUPPORTS_CXX17)
    set( CMAKE_CXX_STANDARD 17 )
  elseif(COMPILER_SUPPORTS_CXX14)
    set( CMAKE_CXX_STANDARD 14 )
  elseif(COMPILER_SUPPORTS_CXX11)
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

if ( IDE_VERSION VERSION_LESS_EQUAL         "3.2.81" )
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
endif()
