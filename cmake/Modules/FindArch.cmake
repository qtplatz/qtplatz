#find target arch

if ( NOT arch_FOUND )

  include(CheckSymbolExists)

  if( WIN32 )
    
    check_symbol_exists( "_M_AMD64" "" RTC_ARCH_X64 )
    
    if ( NOT ARCH_X64 )
      check_symbol_exists( "_M_IX86" "" RTC_ARCH_X86 )
    endif()
    
    check_symbol_exists( "__i386__" "" RTC_ARCH_X86 )
    check_symbol_exists( "__x86_64__" "" RTC_ARCH_X64 )
    
  else()
    
    check_symbol_exists( "__arm__" "" RTC_ARCH_ARM )
    
  endif()

  if ( RTC_ARCH_X64 )
    set( arch_FOUND "x86_64" )
  elseif( RTC_ARCH_X86 )
    set( arch_FOUND "x86" )
  elseif( RTC_ARCH_ARM )
    set( arch_FOUND "arm" )
  endif()

endif()