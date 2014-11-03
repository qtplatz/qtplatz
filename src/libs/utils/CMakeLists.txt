#
# CMakeLists for 'qtplatz/src/libs/utils'
#
cmake_policy(SET CMP0022 NEW)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package( Qt5 REQUIRED Widgets Gui Network Core Script )

if (${dll})
  add_definitions(-DQTCREATOR_UTILS_LIB)
endif()

if (WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4250 /wd4290")
endif(WIN32)

set( utils_SOURCES 
#     environment.cpp
#     environmentmodel.cpp
#     qtcprocess.cpp
     reloadpromptutils.cpp
     settingsselector.cpp
     stringutils.cpp
     textfieldcheckbox.cpp
     textfieldcombobox.cpp
     filesearch.cpp
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
     styledbar.cpp
     stylehelper.cpp
     iwelcomepage.cpp
     fancymainwindow.cpp
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
     json.cpp
     portlist.cpp
     tcpportsgatherer.cpp
     appmainwindow.cpp
     sleep.cpp
     basetreeview.cpp
     qtcassert.cpp
     elfreader.cpp
     bracematcher.cpp
     proxyaction.cpp
     elidinglabel.cpp
     hostosinfo.cpp
     tooltip/tooltip.cpp
     tooltip/tips.cpp
     tooltip/tipcontents.cpp
     unixutils.cpp
     ansiescapecodehandler.cpp
     execmenu.cpp
     completinglineedit.cpp
     winutils.cpp
     itemviews.cpp
     treeviewcombobox.cpp
     proxycredentialsdialog.cpp
)

set( RESOURCES utils.qrc )

if (${dll})
  add_library( utils SHARED ${utils_SOURCES} )
  target_link_libraries( utils LINK_PUBLIC  Qt5::Widgets Qt5::PrintSupport )
else()
  add_library( utils ${utils_SOURCES} )
endif()

