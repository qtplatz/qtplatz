
include( "qtplatz-version" )

set( CPACK_PACKAGE_NAME "qtplatz" )
set( CPACK_PACKAGE_VENDOR "MS-Cheminfo.com" )
set( CPACK_PACKAGE_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO}.${VERSION_PATCH} )
set( CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR} )
set( CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR} )
set( CPACK_PACKAGE_VERSION_PATCH ${VERSION_MICRO} )
set( CPACK_RESOURCE_FILE_LICENSE ${QTPLATZ_SOURCE_DIR}/license.txt )

set( CPACK_WIX_UPGRADE_GUID "10E528B2-6F93-4CFB-91CD-493C6B3D5650" )
include( CPackComponent )

cpack_add_component( applications      DISPLAY_NAME "Applications" GROUP Runtime )
cpack_add_component( translations      DIAPLAY_NAME "Translation files" GROUP Runtime )
cpack_add_component( runtime_libraries DISPLAY_NAME "Runtime libraries" DESCRIPTION "libraries" GROUP Runtime )
cpack_add_component( plugins           DISPLAY_NAME "Plugins" GROUP Runtime )
cpack_add_component( libraries         DISPLAY_NAME "Development libraries"
  DESCRIPTION "Static and import libraries" GROUP Development )
cpack_add_component( headers           DISPLAY_NAME "C++ Headers"
  DESCRIPTION "C++ header files for use with QtPlatz Toolkit" GROUP Development )

cpack_add_component_group( Runtime )
cpack_add_component_group( Development )

set( CPACK_COMPONENT_GROUP_DEVELOPMENT_PARENT_GROUP "Runtime")
set( CPACK_ALL_INSTALL_TYPES Full Developer)
set( CPACK_COMPONENT_LIBRARIES_INSTALL_TYPES Developer Full )
set( CPACK_COMPONENT_HEADERS_INSTALL_TYPES Developer Full )
set( CPACK_COMPONENT_APPLICATIONS_INSTALL_TYPES Full )

set( CPACK_PACKAGE_EXECUTABLES qtplatz "QtPlatz" )
set( CPACK_CREATE_DESKTOP_LINKS qtplatz )

