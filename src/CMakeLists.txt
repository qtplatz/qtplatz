
add_subdirectory( libs )

if ( NOT RTC_ARCH_ARM )
  add_subdirectory( tools )
endif()
