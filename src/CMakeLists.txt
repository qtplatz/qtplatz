
add_subdirectory( libs )
#add_subdirectory( app )
add_subdirectory( adplugins )
add_subdirectory( plugins )

if ( NOT RTC_ARCH_ARM )
  add_subdirectory( tools )
endif()
