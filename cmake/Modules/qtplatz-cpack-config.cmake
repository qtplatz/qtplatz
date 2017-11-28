
include( "version" )

set( CPACK_PACKAGE_VENDOR "MS-Cheminformatics LLC" )
set( CPACK_PACKAGE_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${VERSION_TWEAK} )
set( CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR} )
set( CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR} )
set( CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH} )
set( CPACK_RESOURCE_FILE_LICENSE ${QTPLATZ_SOURCE_DIR}/license.rtf )

include( CPackComponent )

cpack_add_component( applications      DISPLAY_NAME "Applications" GROUP Runtime )
cpack_add_component( translations      DIAPLAY_NAME "Translation files" GROUP Runtime )
cpack_add_component( runtime_libraries DISPLAY_NAME "Runtime libraries" DESCRIPTION "libraries" GROUP Runtime )
cpack_add_component( plugins           DISPLAY_NAME "Plugins" GROUP Runtime )
cpack_add_component( modules           DISPLAY_NAME "Modules" GROUP Runtime )

if ( ${CMAKE_SYSTEM} MATCHES "Linux" )
  cpack_add_component( libraries       DISPLAY_NAME "Development libraries"
    DESCRIPTION "Static and import libraries" GROUP Development )
  cpack_add_component( headers         DISPLAY_NAME "C++ Headers"
    DESCRIPTION "C++ header files for use with QtPlatz Toolkit" GROUP Development )
endif()

cpack_add_component_group( Runtime )

if ( ${CMAKE_SYSTEM} MATCHES "Linux" )
  cpack_add_component_group( Development )
endif()

set( CPACK_ALL_INSTALL_TYPES Full Developer)
set( CPACK_COMPONENT_LIBRARIES_INSTALL_TYPES Developer Full )
set( CPACK_COMPONENT_HEADERS_INSTALL_TYPES Developer Full )
set( CPACK_COMPONENT_APPLICATIONS_INSTALL_TYPES Full )

set( CPACK_PACKAGE_EXECUTABLES qtplatz "qtplatz" )
set( CPACK_CREATE_DESKTOP_LINKS QtPlatz )

