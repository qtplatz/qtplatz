dll {
    DEFINES += QTCREATOR_UTILS_LIB
} else {
    DEFINES += QTCREATOR_UTILS_STATIC_LIB
}

QT += script network

CONFIG += exceptions # used by portlist.cpp, textfileformat.cpp, and ssh/*

win32-msvc* {
    # disable warnings caused by botan headers
    QMAKE_CXXFLAGS += -wd4250 -wd4290
}

SOURCES += $$PWD/environment.cpp \
    $$PWD/environmentmodel.cpp \
    $$PWD/qtcprocess.cpp \
    $$PWD/reloadpromptutils.cpp \
    $$PWD/settingsselector.cpp \
    $$PWD/stringutils.cpp \
    $$PWD/textfieldcheckbox.cpp \
    $$PWD/textfieldcombobox.cpp \
    $$PWD/filesearch.cpp \
    $$PWD/pathchooser.cpp \
    $$PWD/pathlisteditor.cpp \
    $$PWD/wizard.cpp \
    $$PWD/filewizardpage.cpp \
    $$PWD/filesystemwatcher.cpp \
    $$PWD/projectintropage.cpp \
    $$PWD/filenamevalidatinglineedit.cpp \
    $$PWD/projectnamevalidatinglineedit.cpp \
    $$PWD/codegeneration.cpp \
    $$PWD/newclasswidget.cpp \
    $$PWD/classnamevalidatinglineedit.cpp \
    $$PWD/linecolumnlabel.cpp \
    $$PWD/fancylineedit.cpp \
    $$PWD/qtcolorbutton.cpp \
    $$PWD/savedaction.cpp \
    $$PWD/synchronousprocess.cpp \
    $$PWD/savefile.cpp \
    $$PWD/fileutils.cpp \
    $$PWD/textfileformat.cpp \
    $$PWD/consoleprocess.cpp \
    $$PWD/uncommentselection.cpp \
    $$PWD/parameteraction.cpp \
    $$PWD/headerviewstretcher.cpp \
    $$PWD/checkablemessagebox.cpp \
    $$PWD/styledbar.cpp \
    $$PWD/stylehelper.cpp \
    $$PWD/iwelcomepage.cpp \
    $$PWD/fancymainwindow.cpp \
    $$PWD/detailsbutton.cpp \
    $$PWD/detailswidget.cpp \
    $$PWD/changeset.cpp \
    $$PWD/faketooltip.cpp \
    $$PWD/htmldocextractor.cpp \
    $$PWD/navigationtreeview.cpp \
    $$PWD/crumblepath.cpp \
    $$PWD/historycompleter.cpp \
    $$PWD/buildablehelperlibrary.cpp \
    $$PWD/annotateditemdelegate.cpp \
    $$PWD/fileinprojectfinder.cpp \
    $$PWD/ipaddresslineedit.cpp \
    $$PWD/statuslabel.cpp \
    $$PWD/outputformatter.cpp \
    $$PWD/flowlayout.cpp \
    $$PWD/networkaccessmanager.cpp \
    $$PWD/persistentsettings.cpp \
    $$PWD/completingtextedit.cpp \
    $$PWD/json.cpp \
    $$PWD/portlist.cpp \
    $$PWD/tcpportsgatherer.cpp \
    $$PWD/appmainwindow.cpp \
    $$PWD/sleep.cpp \
    $$PWD/basetreeview.cpp \
    $$PWD/qtcassert.cpp \
    $$PWD/elfreader.cpp \
    $$PWD/bracematcher.cpp \
    $$PWD/proxyaction.cpp \
    $$PWD/elidinglabel.cpp \
    $$PWD/hostosinfo.cpp \
    $$PWD/tooltip/tooltip.cpp \
    $$PWD/tooltip/tips.cpp \
    $$PWD/tooltip/tipcontents.cpp \
    $$PWD/unixutils.cpp \
    $$PWD/ansiescapecodehandler.cpp \
    $$PWD/execmenu.cpp \
    $$PWD/completinglineedit.cpp \
    $$PWD/winutils.cpp \
    $$PWD/itemviews.cpp \
    $$PWD/treeviewcombobox.cpp \
    $$PWD/proxycredentialsdialog.cpp

win32:SOURCES += $$PWD/consoleprocess_win.cpp
else:SOURCES += $$PWD/consoleprocess_unix.cpp

HEADERS += \
    $$PWD/environment.h \
    $$PWD/environmentmodel.h \
    $$PWD/qtcprocess.h \
    $$PWD/utils_global.h \
    $$PWD/reloadpromptutils.h \
    $$PWD/settingsselector.h \
    $$PWD/stringutils.h \
    $$PWD/textfieldcheckbox.h \
    $$PWD/textfieldcombobox.h \
    $$PWD/filesearch.h \
    $$PWD/listutils.h \
    $$PWD/pathchooser.h \
    $$PWD/pathlisteditor.h \
    $$PWD/wizard.h \
    $$PWD/filewizardpage.h \
    $$PWD/filesystemwatcher.h \
    $$PWD/projectintropage.h \
    $$PWD/filenamevalidatinglineedit.h \
    $$PWD/projectnamevalidatinglineedit.h \
    $$PWD/codegeneration.h \
    $$PWD/newclasswidget.h \
    $$PWD/classnamevalidatinglineedit.h \
    $$PWD/linecolumnlabel.h \
    $$PWD/fancylineedit.h \
    $$PWD/qtcolorbutton.h \
    $$PWD/savedaction.h \
    $$PWD/consoleprocess.h \
    $$PWD/consoleprocess_p.h \
    $$PWD/synchronousprocess.h \
    $$PWD/savefile.h \
    $$PWD/fileutils.h \
    $$PWD/textfileformat.h \
    $$PWD/uncommentselection.h \
    $$PWD/parameteraction.h \
    $$PWD/headerviewstretcher.h \
    $$PWD/checkablemessagebox.h \
    $$PWD/qtcassert.h \
    $$PWD/styledbar.h \
    $$PWD/stylehelper.h \
    $$PWD/iwelcomepage.h \
    $$PWD/fancymainwindow.h \
    $$PWD/detailsbutton.h \
    $$PWD/detailswidget.h \
    $$PWD/changeset.h \
    $$PWD/faketooltip.h \
    $$PWD/htmldocextractor.h \
    $$PWD/navigationtreeview.h \
    $$PWD/crumblepath.h \
    $$PWD/historycompleter.h \
    $$PWD/buildablehelperlibrary.h \
    $$PWD/annotateditemdelegate.h \
    $$PWD/fileinprojectfinder.h \
    $$PWD/ipaddresslineedit.h \
    $$PWD/statuslabel.h \
    $$PWD/outputformatter.h \
    $$PWD/outputformat.h \
    $$PWD/flowlayout.h \
    $$PWD/networkaccessmanager.h \
    $$PWD/persistentsettings.h \
    $$PWD/completingtextedit.h \
    $$PWD/json.h \
    $$PWD/multitask.h \
    $$PWD/runextensions.h \
    $$PWD/portlist.h \
    $$PWD/tcpportsgatherer.h \
    $$PWD/appmainwindow.h \
    $$PWD/sleep.h \
    $$PWD/basetreeview.h \
    $$PWD/elfreader.h \
    $$PWD/bracematcher.h \
    $$PWD/proxyaction.h \
    $$PWD/hostosinfo.h \
    $$PWD/osspecificaspects.h \
    $$PWD/elidinglabel.h \
    $$PWD/tooltip/tooltip.h \
    $$PWD/tooltip/tips.h \
    $$PWD/tooltip/tipcontents.h \
    $$PWD/tooltip/reuse.h \
    $$PWD/tooltip/effects.h \
    $$PWD/unixutils.h \
    $$PWD/qtcoverride.h \
    $$PWD/ansiescapecodehandler.h \
    $$PWD/execmenu.h \
    $$PWD/completinglineedit.h \
    $$PWD/logging.h \
    $$PWD/winutils.h \
    $$PWD/itemviews.h \
    $$PWD/treeviewcombobox.h \
    $$PWD/scopedswap.h \
    $$PWD/algorithm.h \
    $$PWD/QtConcurrentTools \
    $$PWD/proxycredentialsdialog.h

FORMS += $$PWD/filewizardpage.ui \
    $$PWD/projectintropage.ui \
    $$PWD/newclasswidget.ui \
    $$PWD/proxycredentialsdialog.ui

RESOURCES += $$PWD/utils.qrc
