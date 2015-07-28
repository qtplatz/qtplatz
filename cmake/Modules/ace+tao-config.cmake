
add_library( CORBA::ACE SHARED IMPORTED )
add_library( CORBA::TAO SHARED IMPORTED )
add_library( CORBA::TAO_Utils SHARED IMPORTED )
add_library( CORBA::TAO_PI SHARED IMPORTED )
add_library( CORBA::TAO_PortableServer SHARED IMPORTED )
add_library( CORBA::TAO_AnyTypeCode SHARED IMPORTED )

include( soname )

if (WIN32)

  set_target_properties( CORBA::ACE PROPERTIES
    IMPORTED_IMPLIB       ${ACE+TAO_LIBRARY_DIR}/ACE.lib
    IMPORTED_IMPLIB_DEBUG ${ACE+TAO_LIBRARY_DIR}/ACEd.lib )

  set_target_properties( CORBA::TAO PROPERTIES
    IMPORTED_IMPLIB       ${ACE+TAO_LIBRARY_DIR}/TAO.lib
    IMPORTED_IMPLIB_DEBUG ${ACE+TAO_LIBRARY_DIR}/TAOd.lib )

  set_target_properties( CORBA::TAO_Utils PROPERTIES
    IMPORTED_IMPLIB       ${ACE+TAO_LIBRARY_DIR}/TAO_Utils.lib
    IMPORTED_IMPLIB_DEBUG ${ACE+TAO_LIBRARY_DIR}/TAO_Utilsd.lib )

  set_target_properties( CORBA::TAO_PI PROPERTIES
    IMPORTED_IMPLIB       ${ACE+TAO_LIBRARY_DIR}/TAO_PI.lib
    IMPORTED_IMPLIB_DEBUG ${ACE+TAO_LIBRARY_DIR}/TAO_PId.lib )

  set_target_properties( CORBA::TAO_PortableServer PROPERTIES
    IMPORTED_IMPLIB       ${ACE+TAO_LIBRARY_DIR}/TAO_PortableServer.lib
    IMPORTED_IMPLIB_DEBUG ${ACE+TAO_LIBRARY_DIR}/TAO_PortableServerd.lib )

  set_target_properties( CORBA::TAO_AnyTypeCode PROPERTIES
    IMPORTED_IMPLIB       ${ACE+TAO_LIBRARY_DIR}/TAO_AnyTypeCode.lib
    IMPORTED_IMPLIB_DEBUG ${ACE+TAO_LIBRARY_DIR}/TAO_AnyTypeCoded.lib )

else()

  set_target_properties( CORBA::ACE PROPERTIES
    IMPORTED_LOCATION       ${ACE+TAO_LIBRARY_DIR}/libACE.${SO} )

  set_target_properties( CORBA::TAO PROPERTIES
    IMPORTED_LOCATION       ${ACE+TAO_LIBRARY_DIR}/libTAO.${SO} )

  set_target_properties( CORBA::TAO_Utils PROPERTIES
    IMPORTED_LOCATION       ${ACE+TAO_LIBRARY_DIR}/libTAO_Utils.${SO} )

  set_target_properties( CORBA::TAO_PI PROPERTIES
    IMPORTED_LOCATION       ${ACE+TAO_LIBRARY_DIR}/libTAO_PI.${SO} )

  set_target_properties( CORBA::TAO_PortableServer PROPERTIES
    IMPORTED_LOCATION       ${ACE+TAO_LIBRARY_DIR}/libTAO_PortableServer.${SO} )

  set_target_properties( CORBA::TAO_AnyTypeCode PROPERTIES
    IMPORTED_LOCATION       ${ACE+TAO_LIBRARY_DIR}/libTAO_AnyTypeCode.${SO} )

endif()

