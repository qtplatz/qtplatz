
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

if ( WITH_QT5 AND NOT CMAKE_CROSSCOMPILING )
  add_subdirectory( adplugins )
  add_subdirectory( plugins )

  ############# QtCreator3 base ###############
  if ( IDE_VERSION VERSION_LESS "8.0.0" )
    add_subdirectory( app )
  endif()

endif()

add_subdirectory( tools )

configure_file(
  ${CMAKE_SOURCE_DIR}/cmake/Modules/QtPlatzConfig.cmake.in
  ${CMAKE_BINARY_DIR}/cmake/QtPlatzConfig.cmake
  @ONLY
)

configure_file(
  ${CMAKE_SOURCE_DIR}/cmake/Modules/qtplatz_api.cmake.in
  ${CMAKE_BINARY_DIR}/cmake/qtplatz_api.cmake
  @ONLY
)

export( EXPORT QtPlatzTargets
  FILE ${CMAKE_BINARY_DIR}/cmake/QtPlatzTargets.cmake )

file( COPY
  ${CMAKE_SOURCE_DIR}/boost.cmake
  DESTINATION ${CMAKE_BINARY_DIR}/cmake
)
