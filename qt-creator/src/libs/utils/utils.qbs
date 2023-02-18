import qbs 1.0
import qbs.FileInfo

Project {
    name: "Utils"

    QtcLibrary {
        property bool useNewMimeDatabase: true

        cpp.includePaths: base.concat((useNewMimeDatabase ? "mimetypes2" : "mimetypes"), ".")
        cpp.defines: base.concat([
            "UTILS_LIBRARY"
        ])
        cpp.dynamicLibraries: {
            var libs = [];
            if (qbs.targetOS.contains("windows")) {
                libs.push("user32", "iphlpapi", "ws2_32", "shell32", "ole32");
                if (qbs.toolchainType === "mingw")
                    libs.push("uuid");
                else if (qbs.toolchainType === "msvc")
                    libs.push("dbghelp");
            } else if (qbs.targetOS.contains("unix")) {
                if (!qbs.targetOS.contains("macos"))
                    libs.push("X11");
                if (!qbs.targetOS.contains("openbsd"))
                    libs.push("pthread");
            }
            return libs;
        }

        cpp.enableExceptions: true

        Properties {
            condition: qbs.targetOS.contains("macos")
            cpp.frameworks: ["Foundation", "AppKit"]
        }

        Depends { name: "Qt"; submodules: ["concurrent", "core-private", "network", "qml", "widgets", "xml"] }
        Depends { name: "Qt.macextras"; condition: Qt.core.versionMajor < 6 && qbs.targetOS.contains("macos") }
        Depends { name: "app_version_header" }

        files: [
            "QtConcurrentTools",
            "algorithm.h",
            "ansiescapecodehandler.cpp",
            "ansiescapecodehandler.h",
            "appmainwindow.cpp",
            "appmainwindow.h",
            "archive.cpp",
            "archive.h",
            "aspects.cpp",
            "aspects.h",
            "basetreeview.cpp",
            "basetreeview.h",
            "benchmarker.cpp",
            "benchmarker.h",
            "buildablehelperlibrary.cpp",
            "buildablehelperlibrary.h",
            "camelcasecursor.cpp",
            "camelcasecursor.h",
            "categorysortfiltermodel.cpp",
            "categorysortfiltermodel.h",
            "changeset.cpp",
            "changeset.h",
            "checkablemessagebox.cpp",
            "checkablemessagebox.h",
            "classnamevalidatinglineedit.cpp",
            "classnamevalidatinglineedit.h",
            "codegeneration.cpp",
            "codegeneration.h",
            "commandline.cpp",
            "commandline.h",
            "completinglineedit.cpp",
            "completinglineedit.h",
            "completingtextedit.cpp",
            "completingtextedit.h",
            "cpplanguage_details.h",
            "crumblepath.cpp",
            "crumblepath.h",
            "delegates.cpp",
            "delegates.h",
            "detailsbutton.cpp",
            "detailsbutton.h",
            "detailswidget.cpp",
            "detailswidget.h",
            "devicefileaccess.cpp",
            "devicefileaccess.h",
            "deviceshell.cpp",
            "deviceshell.h",
            "differ.cpp",
            "differ.h",
            "displayname.cpp",
            "displayname.h",
            "dropsupport.cpp",
            "dropsupport.h",
            "elfreader.cpp",
            "elfreader.h",
            "elidinglabel.cpp",
            "elidinglabel.h",
            "environment.cpp",
            "environment.h",
            "environmentdialog.cpp",
            "environmentdialog.h",
            "environmentmodel.cpp",
            "environmentmodel.h",
            "execmenu.cpp",
            "execmenu.h",
            "executeondestruction.h",
            "fadingindicator.cpp",
            "fadingindicator.h",
            "faketooltip.cpp",
            "faketooltip.h",
            "fancylineedit.cpp",
            "fancylineedit.h",
            "fancymainwindow.cpp",
            "fancymainwindow.h",
            "filecrumblabel.cpp",
            "filecrumblabel.h",
            "fileinprojectfinder.cpp",
            "fileinprojectfinder.h",
            "filenamevalidatinglineedit.cpp",
            "filenamevalidatinglineedit.h",
            "filepath.cpp",
            "filepath.h",
            "filesearch.cpp",
            "filesearch.h",
            "filesystemmodel.cpp",
            "filesystemmodel.h",
            "filesystemwatcher.cpp",
            "filesystemwatcher.h",
            "fileutils.cpp",
            "fileutils.h",
            "filewizardpage.cpp",
            "filewizardpage.h",
            "fixedsizeclicklabel.cpp",
            "fixedsizeclicklabel.h",
            "flowlayout.cpp",
            "flowlayout.h",
            "functiontraits.h",
            "futuresynchronizer.cpp",
            "futuresynchronizer.h",
            "fuzzymatcher.cpp",
            "fuzzymatcher.h",
            "globalfilechangeblocker.cpp",
            "globalfilechangeblocker.h",
            "guard.cpp",
            "guard.h",
            "highlightingitemdelegate.cpp",
            "highlightingitemdelegate.h",
            "historycompleter.cpp",
            "historycompleter.h",
            "hostosinfo.h",
            "hostosinfo.cpp",
            "htmldocextractor.cpp",
            "htmldocextractor.h",
            "icon.cpp",
            "icon.h",
            "id.cpp",
            "id.h",
            "indexedcontainerproxyconstiterator.h",
            "infobar.cpp",
            "infobar.h",
            "infolabel.cpp",
            "infolabel.h",
            "itemviews.cpp",
            "itemviews.h",
            "json.cpp",
            "json.h",
            "jsontreeitem.cpp",
            "jsontreeitem.h",
            "launcherinterface.cpp",
            "launcherinterface.h",
            "launcherpackets.cpp",
            "launcherpackets.h",
            "launchersocket.cpp",
            "launchersocket.h",
            "layoutbuilder.cpp",
            "layoutbuilder.h",
            "linecolumn.cpp",
            "linecolumn.h",
            "link.cpp",
            "link.h",
            "listmodel.h",
            "listutils.h",
            "macroexpander.cpp",
            "macroexpander.h",
            "mapreduce.h",
            "mimeutils.h",
            "minimizableinfobars.cpp",
            "minimizableinfobars.h",
            "multitextcursor.cpp",
            "multitextcursor.h",
            "namevaluedictionary.cpp",
            "namevaluedictionary.h",
            "namevalueitem.cpp",
            "namevalueitem.h",
            "namevaluemodel.cpp",
            "namevaluemodel.h",
            "namevaluesdialog.cpp",
            "namevaluesdialog.h",
            "namevaluevalidator.cpp",
            "namevaluevalidator.h",
            "navigationtreeview.cpp",
            "navigationtreeview.h",
            "networkaccessmanager.cpp",
            "networkaccessmanager.h",
            "osspecificaspects.h",
            "outputformat.h",
            "outputformatter.cpp",
            "outputformatter.h",
            "overlaywidget.cpp",
            "overlaywidget.h",
            "overridecursor.cpp",
            "overridecursor.h",
            "parameteraction.cpp",
            "parameteraction.h",
            "pathchooser.cpp",
            "pathchooser.h",
            "pathlisteditor.cpp",
            "pathlisteditor.h",
            "persistentsettings.cpp",
            "persistentsettings.h",
            "predicates.h",
            "pointeralgorithm.h",
            "port.cpp",
            "port.h",
            "portlist.cpp",
            "portlist.h",
            "processenums.h",
            "processhandle.cpp",
            "processhandle.h",
            "processinfo.cpp",
            "processinfo.h",
            "processinterface.cpp",
            "processinterface.h",
            "processreaper.cpp",
            "processreaper.h",
            "processutils.cpp",
            "processutils.h",
            "progressindicator.cpp",
            "progressindicator.h",
            "projectintropage.cpp",
            "projectintropage.h",
            "proxyaction.cpp",
            "proxyaction.h",
            "proxycredentialsdialog.cpp",
            "proxycredentialsdialog.h",
            "proxycredentialsdialog.ui",
            "qrcparser.cpp",
            "qrcparser.h",
            "qtcassert.cpp",
            "qtcassert.h",
            "qtcolorbutton.cpp",
            "qtcolorbutton.h",
            "qtcprocess.cpp",
            "qtcprocess.h",
            "qtcsettings.cpp",
            "qtcsettings.h",
            "reloadpromptutils.cpp",
            "reloadpromptutils.h",
            "removefiledialog.cpp",
            "removefiledialog.h",
            "runextensions.cpp",
            "runextensions.h",
            "savefile.cpp",
            "savefile.h",
            "scopedswap.h",
            "set_algorithm.h",
            "settingsaccessor.cpp",
            "settingsaccessor.h",
            "settingsselector.cpp",
            "settingsselector.h",
            "settingsutils.h",
            "singleton.cpp",
            "singleton.h",
            "sizedarray.h",
            "smallstring.h",
            "smallstringiterator.h",
            "smallstringio.h",
            "smallstringliteral.h",
            "smallstringlayout.h",
            "smallstringmemory.h",
            "smallstringvector.h",
            "sortfiltermodel.h",
            "span.h",
            "../3rdparty/span/span.hpp",
            "statuslabel.cpp",
            "statuslabel.h",
            "stringutils.cpp",
            "stringutils.h",
            "styledbar.cpp",
            "styledbar.h",
            "stylehelper.cpp",
            "stylehelper.h",
            "templateengine.cpp",
            "templateengine.h",
            "temporarydirectory.cpp",
            "temporarydirectory.h",
            "temporaryfile.cpp",
            "temporaryfile.h",
            "terminalcommand.cpp",
            "terminalcommand.h",
            "terminalprocess.cpp",
            "terminalprocess_p.h",
            "textfieldcheckbox.cpp",
            "textfieldcheckbox.h",
            "textfieldcombobox.cpp",
            "textfieldcombobox.h",
            "textfileformat.cpp",
            "textfileformat.h",
            "textutils.cpp",
            "textutils.h",
            "threadutils.cpp",
            "threadutils.h",
            "treemodel.cpp",
            "treemodel.h",
            "treeviewcombobox.cpp",
            "treeviewcombobox.h",
            "headerviewstretcher.cpp",
            "headerviewstretcher.h",
            "uncommentselection.cpp",
            "uncommentselection.h",
            "unixutils.cpp",
            "unixutils.h",
            "url.cpp",
            "url.h",
            "utils.qrc",
            "utils_global.h",
            "utilsicons.h",
            "utilsicons.cpp",
            "utilstr.h",
            "variablechooser.cpp",
            "variablechooser.h",
            "winutils.cpp",
            "winutils.h",
            "wizard.cpp",
            "wizard.h",
            "wizardpage.cpp",
            "wizardpage.h",
            "images/*.png",
        ]

        Group {
            name: "FSEngine"
            prefix: "fsengine/"
            cpp.defines: outer.concat("QTC_UTILS_WITH_FSENGINE")
            files: [
                "diriterator.h",
                "fileiconprovider.cpp",
                "fileiconprovider.h",
                "fileiteratordevicesappender.h",
                "fixedlistfsengine.h",
                "fsengine.cpp",
                "fsengine.h",
                "fsenginehandler.cpp",
                "fsenginehandler.h",
                "fsengine_impl.cpp",
                "fsengine_impl.h",
                "rootinjectfsengine.h",
            ]
        }

        Group {
            name: "Theme"
            prefix: "theme/"
            files: [
                "theme.cpp",
                "theme.h",
                "theme_p.h",
            ]
        }

        Group {
            name: "Tooltip"
            prefix: "tooltip/"
            files: [
                "effects.h",
                "tips.cpp",
                "tips.h",
                "tooltip.cpp",
                "tooltip.h",
            ]
        }

        Group {
            name: "FileUtils_macos"
            condition: qbs.targetOS.contains("macos")
            files: [
                "fileutils_mac.h", "fileutils_mac.mm",
            ]
        }

        Group {
            name: "Theme_macos"
            condition: qbs.targetOS.contains("macos")
            prefix: "theme/"
            files: [
                "theme_mac.h", "theme_mac.mm",
            ]
        }

        Group {
            name: "ProcessHandle_macos"
            condition: qbs.targetOS.contains("macos")
            files: [
                "processhandle_mac.mm",
            ]
        }

        Group {
            name: "MimeTypes"
            prefix: useNewMimeDatabase ? "mimetypes2/" : "mimetypes/"
            files: [
                "mimedatabase.cpp",
                "mimedatabase.h",
                "mimedatabase_p.h",
                "mimeglobpattern.cpp",
                "mimeglobpattern_p.h",
                "mimemagicrule.cpp",
                "mimemagicrule_p.h",
                "mimemagicrulematcher.cpp",
                "mimemagicrulematcher_p.h",
                "mimeprovider.cpp",
                "mimeprovider_p.h",
                "mimetype.cpp",
                "mimetype.h",
                "mimetype_p.h",
                "mimetypeparser.cpp",
                "mimetypeparser_p.h",
                "mimeutils.cpp"
            ]
        }

        Group {
            name: "TouchBar support"
            prefix: "touchbar/"
            files: "touchbar.h"
            Group {
                name: "TouchBar implementation"
                condition: qbs.targetOS.contains("macos")
                files: [
                    "touchbar_appdelegate_mac_p.h",
                    "touchbar_mac_p.h",
                    "touchbar_mac.mm",
                    "touchbar_appdelegate_mac.mm",
                ]
            }
            Group {
                name: "TouchBar stub"
                condition: !qbs.targetOS.contains("macos")
                files: "touchbar.cpp"
            }
        }

        Export {
            Depends { name: "Qt"; submodules: ["concurrent", "widgets" ] }
            cpp.includePaths: base.concat(exportingProduct.useNewMimeDatabase ? "mimetypes2"
                                                                              : "mimetypes")
        }
    }
}
