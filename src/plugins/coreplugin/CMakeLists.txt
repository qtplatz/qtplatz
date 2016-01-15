cmake_policy( SET CMP0020 NEW )

project( Core )
set( PROVIDER QtProject )

#if (WIN32)
#  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY         ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/QtProject ) #dll
#  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/QtProject )
#  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/QtProject )
#endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

configure_file(
  "${PROJECT_SOURCE_DIR}/Core.pluginspec"
  "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec"
  )

execute_process( COMMAND ${XMLPATTERNS} -no-format 
  -output ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.json
  ${QTPLATZ_SOURCE_DIR}/src/pluginjsonmetadata.xsl
  ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.pluginspec
  )

find_package( Qt5 CONFIG REQUIRED PrintSupport Network Sql Widgets Gui Core )

add_definitions(  -DCORE_LIBRARY )

if ( WIN32 )
  add_definitions( -D_SCL_SECURE_NO_WARNINGS )
endif()

include_directories(
  ${CMAKE_SOURCE_DIR}/src		# app/app_version.h
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${Qt5PrintSupport_INCLUDE_DIRS}
  ${Qt5Script_INCLUDE_DIRS}
  ${Qt5Sql_INCLUDE_DIRS}
  ${Qt5Gui_INCLUDE_DIRS}
  ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}
  ${Qt5Network_INCLUDE_DIRS}  
  ${Qt5Core_INCLUDE_DIRS}
  )

if ( MSVC )
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4251 /wd4290 /wd4250")
endif()

set( core_SOURCES
  mainwindow.cpp
  editmode.cpp
  iwizardfactory.cpp
  tabpositionindicator.cpp
  fancyactionbar.cpp
  fancytabwidget.cpp
  generalsettings.cpp
  id.cpp
  icontext.cpp
  messagemanager.cpp
  messageoutputwindow.cpp
  outputpane.cpp
  outputwindow.cpp
  vcsmanager.cpp
  statusbarmanager.cpp
  versiondialog.cpp
  editormanager/editorarea.cpp
  editormanager/editormanager.cpp
  editormanager/editorview.cpp
  editormanager/documentmodel.cpp
  editormanager/openeditorsview.cpp
  editormanager/openeditorswindow.cpp
  editormanager/ieditorfactory.cpp
  editormanager/iexternaleditor.cpp
  actionmanager/actionmanager.cpp
  actionmanager/command.cpp
  actionmanager/commandbutton.cpp
  actionmanager/actioncontainer.cpp
  actionmanager/commandsfile.cpp
  dialogs/saveitemsdialog.cpp
  dialogs/newdialog.cpp
  dialogs/settingsdialog.cpp
  actionmanager/commandmappings.cpp
  dialogs/shortcutsettings.cpp
  dialogs/readonlyfilesdialog.cpp
  dialogs/openwithdialog.cpp
  progressmanager/progressmanager.cpp
  progressmanager/progressview.cpp
  progressmanager/progressbar.cpp
  progressmanager/futureprogress.cpp
  statusbarwidget.cpp
  coreplugin.cpp
  variablemanager.cpp
  modemanager.cpp
  basefilewizard.cpp
  basefilewizardfactory.cpp
  generatedfile.cpp
  plugindialog.cpp
  inavigationwidgetfactory.cpp
  navigationwidget.cpp
  manhattanstyle.cpp
  minisplitter.cpp
  styleanimator.cpp
  findplaceholder.cpp
  rightpane.cpp
  sidebar.cpp
  fileiconprovider.cpp
  mimedatabase.cpp
  icore.cpp
  infobar.cpp
  editormanager/ieditor.cpp
  dialogs/ioptionspage.cpp
  settingsdatabase.cpp
  imode.cpp
  editormanager/systemeditor.cpp
#  designmode.cpp
  editortoolbar.cpp
  outputpanemanager.cpp
  navigationsubwidget.cpp
  sidebarwidget.cpp
  externaltool.cpp
  dialogs/externaltoolconfig.cpp
  toolsettings.cpp
  variablechooser.cpp
  mimetypemagicdialog.cpp
  mimetypesettings.cpp
  dialogs/promptoverwritedialog.cpp
  fileutils.cpp
  featureprovider.cpp
  idocument.cpp
  idocumentfactory.cpp
  textdocument.cpp
  documentmanager.cpp
  removefiledialog.cpp
  iversioncontrol.cpp
  dialogs/addtovcsdialog.cpp
  icorelistener.cpp
  ioutputpane.cpp
#  patchtool.cpp
  windowsupport.cpp
  )

if (WIN32)
  set( core_SOURCES ${core_SOURCES} progressmanager/progressmanager_win.cpp )
  set( core_LIBS ${core_LIBS} ole32 user32 )
endif()

if (APPLE)
  include_directories ( /Developer/Headers/FlatCarbon )
  find_library( CARBON_LIBRARY Carbon )
  find_library( APP_SERVICES_LIBRARY ApplicationServices )
  find_library( APP_KIT_LIBRARY AppKit )
  find_library( APP_DISK_ARBITRATION_LIBRARY DiskArbitration )
  find_library( APP_SECURITY_LIBRARY Security )
  find_library( APP_OPENGL_LIBRARY OpenGL )
  find_library( APP_AGL_LIBRARY AGL )
#  mark_as_advanced ( CARBON_LIBRARY  APP_SERVICES_LIBRARY APP_KIT_LIBRARY )
#  set ( EXTRA_LIBS ${CARBON_LIBRARY} ${QUICKTIME_LIBRARY} ${APP_SERVICES_LIBRARY} ${APP_KIT_LIBRARY} )
  mark_as_advanced ( APP_SERVICES_LIBRARY APP_KIT_LIBRARY )
  set ( EXTRA_LIBS ${APP_SERVICES_LIBRARY} ${APP_KIT_LIBRARY} )

  set( core_SOURCES ${core_SOURCES} progressmanager/progressmanager_mac.mm )
  # LIBS += -framework AppKit
  # LIBS  = -framework AppKit -framework DiskArbitration -framework IOKit -framework Security -framework ApplicationServices -framework OpenGL -framework AGL

elseif(UNIX)

  set( core_SOURCES ${core_SOURCES} progressmanager/progressmanager_x11.cpp )
#    IMAGE_SIZE_LIST = 16 24 32 48 64 128 256 512
#    for(imagesize, IMAGE_SIZE_LIST) {
#        eval(image$${imagesize}.files = images/logo/$${imagesize}/QtProject-qtcreator.png)
#        eval(image$${imagesize}.path = $$QTC_PREFIX/share/icons/hicolor/$${imagesize}x$${imagesize}/apps)
#        INSTALLS += image$${imagesize}
#    }
endif()

set( core_HEADERS
  mainwindow.h
  editmode.h
  iwizardfactory.h
  tabpositionindicator.h
  fancyactionbar.h
  fancytabwidget.h
  generalsettings.h
  id.h
  messagemanager.h
  messageoutputwindow.h
  outputpane.h
  outputwindow.h
  vcsmanager.h
  statusbarmanager.h
  editormanager/editorarea.h
  editormanager/editormanager.h
  editormanager/editormanager_p.h
  editormanager/editorview.h
  editormanager/documentmodel.h
  editormanager/openeditorsview.h
  editormanager/openeditorswindow.h
  editormanager/ieditor.h
  editormanager/iexternaleditor.h
  editormanager/ieditorfactory.h
  actionmanager/actioncontainer.h
  actionmanager/actionmanager.h
  actionmanager/command.h
  actionmanager/commandbutton.h
  actionmanager/actionmanager_p.h
  actionmanager/command_p.h
  actionmanager/actioncontainer_p.h
  actionmanager/commandsfile.h
  dialogs/saveitemsdialog.h
  dialogs/newdialog.h
  dialogs/settingsdialog.h
  actionmanager/commandmappings.h
  dialogs/readonlyfilesdialog.h
  dialogs/shortcutsettings.h
  dialogs/openwithdialog.h
  dialogs/ioptionspage.h
  progressmanager/progressmanager_p.h
  progressmanager/progressview.h
  progressmanager/progressbar.h
  progressmanager/futureprogress.h
  progressmanager/progressmanager.h
  icontext.h
  icore.h
  infobar.h
  imode.h
  ioutputpane.h
  coreconstants.h
  iversioncontrol.h
  ifilewizardextension.h
  icorelistener.h
  versiondialog.h
  core_global.h
  statusbarwidget.h
  coreplugin.h
  variablemanager.h
  modemanager.h
  basefilewizard.h
  basefilewizardfactory.h
  generatedfile.h
  plugindialog.h
  inavigationwidgetfactory.h
  navigationwidget.h
  manhattanstyle.h
  minisplitter.h
  styleanimator.h
  findplaceholder.h
  rightpane.h
  sidebar.h
  fileiconprovider.h
  mimedatabase.h
  settingsdatabase.h
  editormanager/systemeditor.h
#  designmode.h
  editortoolbar.h
  outputpanemanager.h
  navigationsubwidget.h
  sidebarwidget.h
  externaltool.h
  dialogs/externaltoolconfig.h
  toolsettings.h
  variablechooser.h
  mimetypemagicdialog.h
  mimetypesettings.h
  dialogs/promptoverwritedialog.h
  fileutils.h
  externaltoolmanager.h
  generatedfile.h
  featureprovider.h
  idocument.h
  idocumentfactory.h
  textdocument.h
  documentmanager.h
  removefiledialog.h
  dialogs/addtovcsdialog.h
  patchtool.h
  windowsupport.h
  )

set( core_FORMS
  dialogs/newdialog.ui
  dialogs/saveitemsdialog.ui
  dialogs/readonlyfilesdialog.ui
  dialogs/openwithdialog.ui
  generalsettings.ui
  dialogs/externaltoolconfig.ui
  mimetypesettingspage.ui
  mimetypemagicdialog.ui
  removefiledialog.ui
  dialogs/addtovcsdialog.ui
  )
  
set( core_RESOURCES
  core.qrc
  fancyactionbar.qrc
  )

# Core.so directory linked with other plugins, so it should not be MODULE

add_library(
  Core SHARED
  ${core_SOURCES}
  ${core_HEADERS}
  ${core_RESOURCES}
)

link_directories(  ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY} )

target_link_libraries(
  Core LINK_PUBLIC
  utils
  extensionsystem
  aggregation
  Qt5::PrintSupport
  Qt5::Sql
  Qt5::Widgets
  Qt5::Gui
  Qt5::Core
  ${MHDVIEWER_EXE_NAME}
  ${EXTRA_LIBS}
)

if ( UNIX )
  set( core_SOURCES ${core_SOURCES} progressmanager/progressmanager_x11.cpp )

  foreach( imagesize 16 24 32 48 64 128 256 512 )
    #eval(image$${imagesize}.files = images/logo/$${imagesize}/QtProject-qtcreator.png)
    #eval(image$${imagesize}.path = $$QTC_PREFIX/share/icons/hicolor/$${imagesize}x$${imagesize}/apps)
    configure_file(
      images/logo/${imagesize}/QtProject-qtcreator.png
      ${CMAKE_BINARY_DIR}/share/icons/hicolor/${imagesize}x${imagesize}/apps
      )
  endforeach( imagesize )

endif()

set_target_properties( ${PROJECT_NAME}
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER} # for WIN32
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER} # for WIN32
  LIBRARY_OUTPUT_DIRECTORY_DEBUG ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}
  LIBRARY_OUTPUT_DIRECTORY_RELEASE ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}
  LIBRARY_OUTPUT_DIRECTORY ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER} )

install( TARGETS Core
  RUNTIME DESTINATION lib/qtplatz/plugins/${PROVIDER} COMPONENT plugins
  LIBRARY DESTINATION lib/qtplatz/plugins/${PROVIDER} COMPONENT plugins
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )

install( FILES
  "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec"
  DESTINATION lib/qtplatz/plugins/QtProject COMPONENT plugins )