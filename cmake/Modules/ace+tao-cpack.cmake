
find_package( ace+tao )

if ( WIN32 )
  set( SO "dll" )
elseif ( APPLE )
  set( SO "dylib" )
else()
  set( SO "so" )
endif()

if ( ace+tao_FOUND )

  install( FILES
    ${ace+tao_DIR}/lib/TAO_Utils.${SO}
    ${ace+tao_DIR}/lib/TAO_PI.${SO}
    ${ace+tao_DIR}/lib/TAO_PortableServer.${SO}
    ${ace+tao_DIR}/lib/TAO_AnyTypeCode.${SO}
    ${ace+tao_DIR}/lib/TAO.${SO}
    ${ace+tao_DIR}/lib/ACE.${SO}
    DESTINATION bin COMPONENT runtime_libraries )

#  install( DIRECTORY ${ACE_INCLUDE_DIR} DESTINATION include COMPONENT headers
#    FILES_MATCHING PATTERN "*.h" PATTERN "*.inl" )
#  install( DIRECTORY ${TAO_INCLUDE_DIR} DESTINATION include COMPONENT headers
#    FILES_MATCHING PATTERN "*.h" PATTERN "*.inl" )
#  install( DIRECTORY ${ACE+TAO_LIBRARY_DIR} DESTINATION lib COMPONENT libraries )
#  install( DIRECTORY ${ace+tao_DIR}/bin DESTINATION bin COMPONENT libraries )

endif()