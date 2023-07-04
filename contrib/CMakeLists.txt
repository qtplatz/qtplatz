
if ( NOT CMAKE_CROSSCOMPILING AND NOT RTC_ARCH_ARM )
  add_subdirectory( socfpga )
endif()

if ( NOT CMAKE_CROSSCOMPILING ) #AND NOT RTC_ARCH_ARM )
  add_subdirectory( accutof )
endif()

if ( NOT CMAKE_CROSSCOMPILING AND NOT RTC_ARCH_ARM )
  # message( STATUS "------------ contrib/agilent ----------------" )
  add_subdirectory( agilent )
endif()

if ( NOT CMAKE_CROSSCOMPILING ) # AND NOT RTC_ARCH_ARM )
  add_subdirectory( infitof )
endif()

if ( NOT CMAKE_CROSSCOMPILING AND NOT RTC_ARCH_ARM )
#  add_subdirectory( shrader )
endif()

if ( NOT CMAKE_CROSSCOMPILING AND NOT RTC_ARCH_ARM AND AqMD3_FOUND )
  add_subdirectory( acqiris )
endif()

if ( WITH_Wolfram AND NOT CMAKE_CROSSCOMPILING AND NOT RTC_ARCH_ARM )
#  add_subdirectory( wolfram )
endif()

if ( ${CMAKE_SYSTEM} MATCHES "Linux" )
  add_subdirectory( linux )
elseif( WIN32 )
  add_subdirectory( windows )
endif()

if (WIN32)
  add_subdirectory( bruker )
  add_subdirectory( installer )
elseif( APPLE )
  add_subdirectory( installer )
endif()

#if ( Boost_PYTHON_FOUND AND Python3_FOUND )
if ( Python3_FOUND )
  if ( VERBOSE )
    message( STATUS "------------ contrib/py_modules ----------------" )
  endif()
  add_subdirectory( py_modules )
endif()
