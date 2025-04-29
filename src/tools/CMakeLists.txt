
if ( WIN32 )
  set ( OPENSSL_ROOT_DIR "C:/Program Files/OpenSSL" )
  set ( OPENSSL_USE_STATIC_LIBS TRUE )
endif()

find_package( OpenSSL )

if ( OPENSSL_FOUND )
  add_subdirectory ( http )
endif()

add_subdirectory ( adurlx )
add_subdirectory ( eventbroker )
if ( APPLE )
  add_subdirectory ( genchromatogram )
endif()

if ( NOT CMAKE_CROSSCOMPILING )

  add_subdirectory ( adfile )
  if ( NOT WIN32 )
    add_subdirectory ( adimport ) #-- issue (direct liniing with adtextfiles .cpp files) need to be resolved
  endif()
  add_subdirectory ( adexport )
  add_subdirectory ( scanlaw )
  if ( rdkit_FOUND )
    add_subdirectory ( molcat )
    if ( APPLE )
      add_subdirectory ( xyz )
    endif()
  endif()

  if ( QT_FOUND AND rdkit_FOUND )

    add_subdirectory ( eventtool )
    add_subdirectory ( sdfview )

    if ( HDF5_FOUND AND 0 )
      add_subdirectory ( counting )
    endif()

    if ( OpenCV_FOUND AND HDF5_FOUND AND 0 )
      add_subdirectory ( counting2d )
    endif()

    if ( NOT WIN32 )
      add_subdirectory ( acqiris )
    endif()

  endif()

endif()
