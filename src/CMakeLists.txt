
set (git_cmd "git")
set (git_arg "describe")

execute_process(COMMAND ${git_cmd} ${git_arg}
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  RESULT_VARIABLE git_result
  OUTPUT_VARIABLE git_describe
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )

configure_file(
  ${PROJECT_SOURCE_DIR}/version.h.in
  ${PROJECT_BINARY_DIR}/version.h
  )

add_subdirectory( libs )
add_subdirectory( app )
add_subdirectory( adplugins )
add_subdirectory( plugins )

if ( NOT RTC_ARCH_ARM )
  add_subdirectory( tools )
endif()

