#find target arch

include(CheckSymbolExists)

if ( arch_FOUND )
  return()
endif()

if( WIN32 )
  
  check_symbol_exists( "_M_AMD64" "" RTC_ARCH_X64 )
  
  if ( NOT RTC_ARCH_X64 )
    check_symbol_exists( "_M_X64" "" RTC_ARCH_X64 )
  endif()

  if ( NOT RTC_ARCH_X64 )
    check_symbol_exists( "_M_IX86" "" RTC_ARCH_X86 )
  endif()

else()  

  check_symbol_exists( "__x86_64__" "" RTC_ARCH_X64 )

  if ( NOT RTC_ARCH_X64 )  
    check_symbol_exists( "__i386__" "" RTC_ARCH_X86 )

    if ( NOT RTC_ARCH_X86 )
      check_symbol_exists( "__arm__" "" RTC_ARCH_ARM )      
    endif()

  endif()
  
endif()

if ( RTC_ARCH_X64 )

  set( arch_FOUND "x86_64" )
  set( __arch "x86_64" )  

elseif( RTC_ARCH_X86 )

  set( arch_FOUND "x86" )
  set( __arch "x86" )

elseif( RTC_ARCH_ARM )

  set( arch_FOUND "arm" )
  set( __arch "armhf" )

endif()

