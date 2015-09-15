#
# CMakeLists for 'qtplatz/src/libs/utils'
#
cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0020 NEW)

project( utils )

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

#find_package( Qt5 REQUIRED Widgets Gui Network Core Script PrintSupport )
find_package( Qt5 REQUIRED Widgets Gui Network Core PrintSupport )

include_directories(
  ${Qt5Network_INCLUDE_DIRS}
  ${Qt5Script_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}
  )

add_definitions(-DQTCREATOR_UTILS_LIB)

if (WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4250 /wd4290")
  add_definitions(-D_SCL_SECURE_NO_WARNINGS)
endif(WIN32)

set( utils_lib_SOURCES
  consoleprocess.cpp
)

if (WIN32)
  SET( utils_lib_SOURCES ${utils_lib_SOURCES}
    consoleprocess_win.cpp
    )
else()
  SET( utils_lib_SOURCES ${utils_lib_SOURCES} consoleprocess_unix.cpp )
endif()

set( utils_SOURCES 
     styledbar.cpp           #--
     stylehelper.cpp         #--
     fancymainwindow.cpp     #--
     tooltip/tooltip.cpp          #--
     tooltip/tips.cpp         #--
     tooltip/tipcontents.cpp         #--
     unixutils.cpp         #--
     environment.cpp
     environmentmodel.cpp
     winutils.cpp
     itemviews.cpp
     treeviewcombobox.cpp
     qtcassert.cpp          #--
     qtcprocess.cpp
     reloadpromptutils.cpp
     settingsselector.cpp
     stringutils.cpp
    textfieldcheckbox.cpp
     textfieldcombobox.cpp
#     filesearch.cpp
     pathchooser.cpp
     pathlisteditor.cpp
     wizard.cpp
     filewizardpage.cpp
     filesystemwatcher.cpp
     projectintropage.cpp
    filenamevalidatinglineedit.cpp
     projectnamevalidatinglineedit.cpp
     codegeneration.cpp
     newclasswidget.cpp
     classnamevalidatinglineedit.cpp
     linecolumnlabel.cpp
    fancylineedit.cpp
    qtcolorbutton.cpp
     savedaction.cpp
     synchronousprocess.cpp
     savefile.cpp
     fileutils.cpp
     textfileformat.cpp
     consoleprocess.cpp
     uncommentselection.cpp
     parameteraction.cpp
     headerviewstretcher.cpp
     checkablemessagebox.cpp
#     iwelcomepage.cpp
     detailsbutton.cpp
     detailswidget.cpp
     changeset.cpp
     faketooltip.cpp
     htmldocextractor.cpp
     navigationtreeview.cpp
     crumblepath.cpp
     historycompleter.cpp
     buildablehelperlibrary.cpp
     annotateditemdelegate.cpp
     fileinprojectfinder.cpp
     ipaddresslineedit.cpp
     statuslabel.cpp
     outputformatter.cpp
     flowlayout.cpp
     networkaccessmanager.cpp
     persistentsettings.cpp
     completingtextedit.cpp
#     json.cpp
     portlist.cpp
#     tcpportsgatherer.cpp
     appmainwindow.cpp
     sleep.cpp
     basetreeview.cpp
     elfreader.cpp
     bracematcher.cpp
     proxyaction.cpp
     elidinglabel.cpp
     hostosinfo.cpp
     ansiescapecodehandler.cpp
     execmenu.cpp
     completinglineedit.cpp
     proxycredentialsdialog.cpp
)

set( utils_RESOURCES utils.qrc )

add_library(
  utils SHARED
  ${utils_SOURCES}
  ${utils_lib_SOURCES}
  ${utils_RESOURCES}
  )

target_link_libraries(
  utils LINK_PUBLIC
  Qt5::Widgets
  Qt5::Network
  Qt5::PrintSupport
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT modules
  ARCHIVE DESTINATION lib/qtplatz COMPONENT runtime_libraries )
