cmake_minimum_required (VERSION 2.8.9)

if ( NOT CMAKE_CROSSCOMPILING AND NOT RTC_ARCH_ARM )
  add_subdirectory( shrader )
endif()

if ( NOT CMAKE_CROSSCOMPILING AND NOT RTC_ARCH_ARM )
  # message( STATUS "------------ contrib/agilent ----------------" )
  add_subdirectory( agilent )
endif()

if ( NOT CMAKE_CROSSCOMPILING AND NOT RTC_ARCH_ARM AND AqMD3_FOUND )
  add_subdirectory( acqiris )
endif()

if ( NOT CMAKE_CROSSCOMPILING AND NOT RTC_ARCH_ARM )
  # message( STATUS "------------ contrib/multum ----------------" )
  add_subdirectory( multum )
endif()

if ( NOT CMAKE_CROSSCOMPILING AND NOT RTC_ARCH_ARM )
  add_subdirectory( infitof )
endif()

if ( NOT CMAKE_CROSSCOMPILING AND NOT RTC_ARCH_ARM )
  add_subdirectory( accutof )
endif()

if ( NOT CMAKE_CROSSCOMPILING AND NOT RTC_ARCH_ARM )
  add_subdirectory( socfpga )
endif()

if ( WITH_Wolfram AND NOT CMAKE_CROSSCOMPILING AND NOT RTC_ARCH_ARM )
  message( STATUS "------------ Wolfram ----------------" )
  add_subdirectory( wolfram )
endif()

if ( ${CMAKE_SYSTEM} MATCHES "Linux" )
  # message( STATUS "------------ contrib/linux ----------------" )
  add_subdirectory( linux )
elseif( WIN32 )
  # message( STATUS "------------ contrib/windows ----------------" )
  add_subdirectory( windows )
endif()

if (WIN32)
  add_subdirectory( bruker )
  add_subdirectory( installer )
elseif( APPLE )
  add_subdirectory( installer )
endif()

if ( WITH_PYTHON3 AND ( NOT CMAKE_CROSSCOMPILING AND NOT RTC_ARCH_ARM ) )
  find_package( Python3 COMPONENTS Interpreter Development )
  if ( Python3_FOUND )
    add_subdirectory( py_modules )
  endif()
endif()
