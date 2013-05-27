TEMPLATE = lib
TARGET = Utils
QT += gui \
    network

DEFINES += QTCREATOR_UTILS_LIB
include(../../qtplatz_library.pri)

SOURCES += \
    pathchooser.cpp \
    filewizardpage.cpp \
    filewizarddialog.cpp \
    basevalidatinglineedit.cpp \
    filenamevalidatinglineedit.cpp \
    qtcolorbutton.cpp \
    treewidgetcolumnstretcher.cpp \
    styledbar.cpp \
    stylehelper.cpp \
    fancymainwindow.cpp

#    reloadpromptutils.cpp \
#    settingsutils.cpp \
#    pathlisteditor.cpp \
#    projectintropage.cpp \
#    projectnamevalidatinglineedit.cpp \
#    codegeneration.cpp \
#    newclasswidget.cpp \
#    classnamevalidatinglineedit.cpp \
#    linecolumnlabel.cpp \
#    fancylineedit.cpp \
#    savedaction.cpp \
#    submiteditorwidget.cpp \
#    synchronousprocess.cpp \
#    submitfieldwidget.cpp \
#    uncommentselection.cpp \
#    parameteraction.cpp \
#    checkablemessagebox.cpp \
#    welcomemodetreewidget.cpp \
#    iwelcomepage.cpp \
#    detailsbutton.cpp \
#    detailswidget.cpp

win32 { 
    SOURCES += \
        winutils.cpp
    HEADERS += winutils.h
}

HEADERS += utils_global.h \
    listutils.h \
    pathchooser.h \
    filewizardpage.h \
    filewizarddialog.h \
    basevalidatinglineedit.h \
    filenamevalidatinglineedit.h \
    qtcolorbutton.h \
    treewidgetcolumnstretcher.h \
    qtcassert.h \
    styledbar.h \
    stylehelper.h \
    fancymainwindow.h
#    reloadpromptutils.h \
#    settingsutils.h \
#    pathlisteditor.h \
#    projectintropage.h \
#    projectnamevalidatinglineedit.h \
#    codegeneration.h \
#    newclasswidget.h \
#    classnamevalidatinglineedit.h \
#    linecolumnlabel.h \
#    fancylineedit.h \
#    savedaction.h \
#    submiteditorwidget.h \
#    abstractprocess.h \
#    synchronousprocess.h \
#    submitfieldwidget.h \
#    uncommentselection.h \
#    parameteraction.h \
#    checkablemessagebox.h \
#    welcomemodetreewidget.h \
#    iwelcomepage.h \
#    detailsbutton.h \
#    detailswidget.h

FORMS += filewizardpage.ui \
    projectintropage.ui \
    newclasswidget.ui \
    submiteditorwidget.ui \
    checkablemessagebox.ui
RESOURCES += utils.qrc
