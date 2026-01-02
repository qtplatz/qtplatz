Git
---

```
git clone https://github.com/qt-creator/qt-creator.git
cd qt-creator
git checkout v13.0.0
```

Build
-----
On Mac
```bash
% cmake -DCMAKE_PREFIX_PATH=/opt/Qt5/5.15.2/lib/cmake ~/src/qt-creator
-- or -- cmake -DCMAKE_PREFIX_PATH=/opt/Qt6/6.4.0/macos/lib/cmake/ ~/src/qt-creator
% make -j8
```
Above command creates "Qt Creator.app" on macOS.

On Linux
```bash
cmake -DCMAKE_PREFIX_PATH=/opt/Qt/6.4.2/gcc_64/lib/cmake/ ~/src/
```

Change IDE output name
----------------------
Change name in ./qt-creator/cmake/QtCreatorIDEBranding.cmake as following: (2026-01-02 update)

```
diff --git a/qt-creator/cmake/QtCreatorIDEBranding.cmake b/qt-creator/cmake/QtCreatorIDEBranding.cmake
index 5fc8a3ab1..89fe23d43 100644
--- a/qt-creator/cmake/QtCreatorIDEBranding.cmake
+++ b/qt-creator/cmake/QtCreatorIDEBranding.cmake
@@ -1,21 +1,24 @@
-set(IDE_VERSION "13.0.0")                             # The IDE version.
-set(IDE_VERSION_COMPAT "13.0.0")                      # The IDE Compatibility version.
-set(IDE_VERSION_DISPLAY "13.0.0")                     # The IDE display version.
-set(IDE_COPYRIGHT_YEAR "2024")                        # The IDE current copyright year.
+set(IDE_VERSION "18.0.82")                            # The IDE version.
+set(IDE_VERSION_COMPAT "18.0.82")                     # The IDE Compatibility version.
+set(IDE_VERSION_DISPLAY "19.0.0-beta1")               # The IDE display version.

 set(IDE_SETTINGSVARIANT "QtProject")                  # The IDE settings variation.
-set(IDE_DISPLAY_NAME "qtplatz")                     # The IDE display name.
-set(IDE_ID "qtplatz")                               # The IDE id (no spaces, lowercase!)
-set(IDE_CASED_ID "QtPlatz")                         # The cased IDE id (no spaces!)
-set(IDE_BUNDLE_IDENTIFIER "com.ms-cheminfo.${IDE_ID}") # The macOS application bundle identifier.
+set(IDE_DISPLAY_NAME "QtPlatz")                       # The IDE display name.
+set(IDE_ID "qtcreator")                               # The IDE id (no spaces, lowercase!)
+set(IDE_CASED_ID "QtPlatz")                           # The cased IDE id (no spaces!)
+set(IDE_BUNDLE_IDENTIFIER "com.ms-cheminfo.${IDE_ID}")# The macOS application bundle identifier.
+set(IDE_APP_ID "com.ms-cheminfo.${IDE_ID}")           # The free desktop application identifier.
+set(IDE_PUBLISHER "MS-Cheminfomatics LLC")
+set(IDE_AUTHOR "${IDE_PUBLISHER} and other contributors.")
+set(IDE_COPYRIGHT "Copyright (C) ${IDE_AUTHOR}")

 set(PROJECT_USER_FILE_EXTENSION .user)
-set(IDE_DOC_FILE "qtplatz/qtplatz.qdocconf")
-set(IDE_DOC_FILE_ONLINE "qtplatz/qtplatz-online.qdocconf")
+set(IDE_DOC_FILE "qtcreator/qtcreator.qdocconf")
+set(IDE_DOC_FILE_ONLINE "qtcreator/qtcreator-online.qdocconf")

 # Absolute, or relative to <qtcreator>/src/app
 # Should contain qtcreator.ico, qtcreator.xcassets
 set(IDE_ICON_PATH "")
 # Absolute, or relative to <qtcreator>/src/app
-# Should contain images/logo/(16|24|32|48|64|128|256|512)/QtProject-qtcreator.png
+# Should contain images/logo/(16|24|32|48|64|128|256|512)/QtProject-${IDE_ID}.png
 set(IDE_LOGO_PATH "")
```

Previous Version (8 and 9)
----

```
diff --git a/cmake/QtCreatorIDEBranding.cmake b/cmake/QtCreatorIDEBranding.cmake
index e7f7b39cad..25bc4e71bf 100644
--- a/cmake/QtCreatorIDEBranding.cmake
+++ b/cmake/QtCreatorIDEBranding.cmake
@@ -4,14 +4,14 @@ set(IDE_VERSION_DISPLAY "9.0.1")                      # The IDE display version.
 set(IDE_COPYRIGHT_YEAR "2022")                        # The IDE current copyright year.

 set(IDE_SETTINGSVARIANT "QtProject")                  # The IDE settings variation.
-set(IDE_DISPLAY_NAME "Qt Creator")                    # The IDE display name.
-set(IDE_ID "qtcreator")                               # The IDE id (no spaces, lowercase!)
-set(IDE_CASED_ID "QtCreator")                         # The cased IDE id (no spaces!)
-set(IDE_BUNDLE_IDENTIFIER "org.qt-project.${IDE_ID}") # The macOS application bundle identifier.
+set(IDE_DISPLAY_NAME "qtplatz")                    # The IDE display name.
+set(IDE_ID "qtplatz")                               # The IDE id (no spaces, lowercase!)
+set(IDE_CASED_ID "QtPlatz")                         # The cased IDE id (no spaces!)
+set(IDE_BUNDLE_IDENTIFIER "com.ms-cheminfo.${IDE_ID}") # The macOS application bundle identifier.

 set(PROJECT_USER_FILE_EXTENSION .user)
-set(IDE_DOC_FILE "qtcreator/qtcreator.qdocconf")
-set(IDE_DOC_FILE_ONLINE "qtcreator/qtcreator-online.qdocconf")
+set(IDE_DOC_FILE "qtplatz/qtplatz.qdocconf")
+set(IDE_DOC_FILE_ONLINE "qtplatz/qtplatz-online.qdocconf")

 # Absolute, or relative to <qtcreator>/src/app
 # Should contain qtcreator.ico, qtcreator.xcassets
```


0. List of directories/files
======================
```
 % find . -name "*.deleted"
./doc.deleted
./tests.deleted
./packaging.deleted
./src/tools/qtcreatorwidgets.deleted
./src/tools/valgrindfake.deleted
./src/tools/icons.deleted
./src/tools/qtcdebugger.deleted
./src/tools/sqlitetester.deleted
./src/tools/qmlpuppet.deleted
./src/tools/qtc-askpass.deleted
./src/tools/sdktool.deleted
./src/tools/3rdparty.deleted
./src/tools/cplusplus-update-frontend.deleted
./src/tools/buildoutputparser.deleted
./src/tools/cplusplustools.qbs.deleted
./src/tools/qtcrashhandler.deleted
./src/tools/iostool.deleted
./src/tools/process_stub.deleted
./src/tools/tools.qbs.deleted
./src/tools/cplusplus-frontend.deleted
./src/tools/perfparser.deleted
./src/tools/disclaim.deleted
./src/tools/cplusplus-shared.deleted
./src/tools/qtpromaker.deleted
./src/tools/cplusplus-mkvisitor.deleted
./src/tools/cplusplus-ast2png.deleted
./src/plugins/ctfvisualizer.deleted
./src/plugins/learning.deleted
./src/plugins/silversearcher.deleted
./src/plugins/perforce.deleted
./src/plugins/bineditor.deleted
./src/plugins/modeleditor.deleted
./src/plugins/qmlprojectmanager.deleted
./src/plugins/qmlprofiler.deleted
./src/plugins/saferenderer.deleted
./src/plugins/extensionmanager.deleted
./src/plugins/designer.deleted
./src/plugins/compilerexplorer.deleted
./src/plugins/boot2qt.deleted
./src/plugins/classview.deleted
./src/plugins/qbsprojectmanager.deleted
./src/plugins/vcsbase.deleted
./src/plugins/emacskeys.deleted
./src/plugins/genericprojectmanager.deleted
./src/plugins/baremetal.deleted
./src/plugins/qmakeprojectmanager.deleted
./src/plugins/terminal.deleted
./src/plugins/clangformat.deleted
./src/plugins/cppcheck.deleted
./src/plugins/cpaster.deleted
./src/plugins/extrapropertyeditormanager.deleted
./src/plugins/effectcomposer.deleted
./src/plugins/updateinfo.deleted
./src/plugins/qmldesignerbase.deleted
./src/plugins/lua.deleted
./src/plugins/languageclient.deleted
./src/plugins/conan.deleted
./src/plugins/clangcodemodel.deleted
./src/plugins/perfprofiler.deleted
./src/plugins/todo.deleted
./src/plugins/debugger.deleted
./src/plugins/glsleditor.deleted
./src/plugins/axivion.deleted
./src/plugins/screenrecorder.deleted
./src/plugins/clangtools.deleted
./src/plugins/squish.deleted
./src/plugins/cppeditor.deleted
./src/plugins/plugins.qbs.deleted
./src/plugins/nim.deleted
./src/plugins/vcpkg.deleted
./src/plugins/gitlab.deleted
./src/plugins/remotelinux.deleted
./src/plugins/qmljseditor.deleted
./src/plugins/coco.deleted
./src/plugins/qnx.deleted
./src/plugins/mesonprojectmanager.deleted
./src/plugins/qmljstools.deleted
./src/plugins/webassembly.deleted
./src/plugins/copilot.deleted
./src/plugins/mercurial.deleted
./src/plugins/scxmleditor.deleted
./src/plugins/mcusupport.deleted
./src/plugins/beautifier.deleted
./src/plugins/help.deleted
./src/plugins/cmakeprojectmanager.deleted
./src/plugins/compilationdatabaseprojectmanager.deleted
./src/plugins/appstatisticsmonitor.deleted
./src/plugins/resourceeditor.deleted
./src/plugins/subversion.deleted
./src/plugins/welcome.deleted
./src/plugins/docker.deleted
./src/plugins/clearcase.deleted
./src/plugins/imageviewer.deleted
./src/plugins/fossil.deleted
./src/plugins/texteditor.deleted
./src/plugins/valgrind.deleted
./src/plugins/git.deleted
./src/plugins/cvs.deleted
./src/plugins/bazaar.deleted
./src/plugins/qtapplicationmanager.deleted
./src/plugins/qmldesigner.deleted
./src/plugins/devcontainer.deleted
./src/plugins/autotest.deleted
./src/plugins/android.deleted
./src/plugins/python.deleted
./src/plugins/fakevim.deleted
./src/plugins/helloworld.deleted
./src/plugins/projectexplorer.deleted
./src/plugins/incredibuild.deleted
./src/plugins/qmlpreview.deleted
./src/plugins/diffeditor.deleted
./src/plugins/macros.deleted
./src/plugins/autotoolsprojectmanager.deleted
./src/plugins/ios.deleted
./src/plugins/qtsupport.deleted
./src/plugins/serialterminal.deleted
./src/shared/designerintegrationv2.deleted
./src/shared/proparser.deleted
./src/shared/qtcreator_pch.h.deleted
./src/shared/cpaster.deleted
./src/shared/qtcreator_gui_pch.h.deleted
./src/shared/registryaccess.deleted
./src/shared/modeltest.deleted
./src/shared/qbs.deleted
./src/shared/help.deleted
./src/shared/json.deleted
./src/libs/qmleditorwidgets.deleted
./src/libs/qmldebug.deleted
./src/libs/modelinglib.deleted
./src/libs/qmlpuppetcommunication.deleted
./src/libs/advanceddockingsystem.deleted
./src/libs/tracing.deleted
./src/libs/glsl.deleted
./src/libs/languageutils.deleted
./src/libs/languageserverprotocol.deleted
./src/libs/sqlite.deleted
./src/libs/cplusplus.deleted
./src/libs/3rdparty/toml11.deleted
./src/libs/3rdparty/sol2.deleted
./src/libs/3rdparty/syntax-highlighting.deleted
./src/libs/3rdparty/sqlite.deleted
./src/libs/3rdparty/cplusplus.deleted
./src/libs/3rdparty/googletest.deleted
./src/libs/3rdparty/yaml-cpp.deleted
./src/libs/3rdparty/json.deleted
./src/libs/devcontainer.deleted
./src/libs/googletest.deleted
./src/libs/qtcreatorcdbext.deleted
```

Exclude unused modules (Previous Version (8 and 9))
======================

1. qt-creator/CMakeLists.txt
---
Exclude line: add_subdirectory(doc)

add_subdirectory(share) <-- not included in patch


2. qt-creator/src/CMakeLists.txt
---
```
diff --git a/src/CMakeLists.txt b/src/CMakeLists.txt
index b567bbe120..0ffcbc085d 100644
--- a/src/CMakeLists.txt
+++ b/src/CMakeLists.txt
@@ -5,11 +5,11 @@ target_include_directories(app_version
 install(TARGETS app_version EXPORT QtCreator)

 add_subdirectory(libs)
-add_subdirectory(share)
+#add_subdirectory(share)
 add_subdirectory(shared)
 add_subdirectory(plugins)
 add_subdirectory(app)
-add_subdirectory(tools)
+#add_subdirectory(tools)

 install(
   FILES
```

3. ~/src/qt-creator/src/share/CMakeLists.txt
---
```
diff --git a/src/share/CMakeLists.txt b/src/share/CMakeLists.txt
index 57850ff783..4975db9644 100644
--- a/src/share/CMakeLists.txt
+++ b/src/share/CMakeLists.txt
@@ -1,2 +1,2 @@
 add_subdirectory(3rdparty)
-add_subdirectory(qtcreator)
+#add_subdirectory(qtcreator)
```

4. ~src/qt-creator/src/libs/CMakeLists.txt
---
```
diff --git a/src/libs/CMakeLists.txt b/src/libs/CMakeLists.txt
index 7265810737..9aaae4c792 100644
--- a/src/libs/CMakeLists.txt
+++ b/src/libs/CMakeLists.txt
@@ -1,9 +1,10 @@
-add_subdirectory(3rdparty)
+#add_subdirectory(3rdparty)

-add_subdirectory(advanceddockingsystem)
+#add_subdirectory(advanceddockingsystem)
 add_subdirectory(aggregation)
 add_subdirectory(extensionsystem)
 add_subdirectory(utils)
+if ( FALSE )
 add_subdirectory(languageutils)
 add_subdirectory(cplusplus)
 add_subdirectory(modelinglib)
@@ -52,3 +53,4 @@ if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/qlitehtml/src/CMakeLists.txt)
     )
   endif()
 endif()
+endif()
```

5. src/shared/CMakeLists.txt
---
```
diff --git a/src/shared/CMakeLists.txt b/src/shared/CMakeLists.txt
index 93409b0bf4..e02563eab8 100644
--- a/src/shared/CMakeLists.txt
+++ b/src/shared/CMakeLists.txt
@@ -1,7 +1,8 @@
-add_subdirectory(designerintegrationv2)
-add_subdirectory(proparser)
+#add_subdirectory(designerintegrationv2)
+#add_subdirectory(proparser)
 add_subdirectory(qtsingleapplication)
 add_subdirectory(qtlockedfile)
+if ( FALSE )
 add_subdirectory(help)
 add_subdirectory(registryaccess)

@@ -30,3 +31,4 @@ if (BUILD_QBS)
   set(QBS_INSTALL_QCH_DOCS ${WITH_DOCS} CACHE BOOL "")
   add_subdirectory(qbs)
 endif()
+endif()
```

6. ~/src/qt-creator/src/plugins/CMakeLists.txt
---
```
diff --git a/src/plugins/CMakeLists.txt b/src/plugins/CMakeLists.txt
index 8051db6e59..117bd9af80 100644
--- a/src/plugins/CMakeLists.txt
+++ b/src/plugins/CMakeLists.txt
@@ -1,6 +1,6 @@
 # Level 0:
 add_subdirectory(coreplugin)
-
+if ( FALSE )
 # Level 1: (only depends of Level 0)
 add_subdirectory(texteditor)
 add_subdirectory(serialterminal)
@@ -97,3 +97,4 @@ add_subdirectory(studiowelcome)
 add_subdirectory(qnx)
 add_subdirectory(webassembly)
 add_subdirectory(mcusupport)
+endif()
```

Delete unsed files
==================

qt-creator
---
```
% cd ~/src/qt-creator
% mkdir deleted
% mv tests deleted
% mv doc deleted
% mkdir -p deleted/src/shared
% mv src/shared/cpaster deleted/src/shared
% mv src/shared/designerintegrationv2 deleted/src/shared
% mv src/shared/help/ deleted/src/shared
% mv src/shared/json deleted/src/shared
% mv src/shared/modeltest deleted/src/shared
% mv src/shared/proparser deleted/src/shared
% mv src/shared/registryaccess deleted/src/shared

% mkdir -p deleted/src/libs/3rdparty
% mv src/libs/3rdparty/cplusplus deleted/src/libs/3rdparty
% mv src/libs/3rdparty/json deleted/src/libs/3rdparty
% mv src/libs/3rdparty/sqlite deleted/src/libs/3rdparty
% mv src/libs/3rdparty/syntax-highlighting deleted/src/libs/3rdparty
% mv src/libs/3rdparty/yaml-cpp deleted/src/libs/3rdparty

% mv qbs deleted
% mv src/shared/pch_files.qbs deleted/src/shared <--- error
% mv src/src.qbs deleted/src

% mv qtcreator.pri deleted
% mv qtcreator.qbs deleted
% mv qtcreator_ide_branding.pri deleted
% mv qtcreator_testvars.pri deleted
% mv .tag deleted
```


2. plugin directory (delete all files; except for CMakeFiles.txt and coreplugin)
----
```
% mkdir -p deleted/src
% mv src/plugins deleted/src
% mkdir src/plugins
% mv deleted/src/plugins/CMakeLists.txt src/plugins
% mv deleted/src/plugins/coreplugin src/plugins
```
```
% mkdir src/plugins/texteditor
% mv deleted/src/plugins/texteditor/blockrange.h src/plugins/texteditor
% mv deleted/src/plugins/texteditor/formatter.h src/plugins/texteditor
% mv deleted/src/plugins/texteditor/indenter.h src/plugins/texteditor
% mv deleted/src/plugins/texteditor/refactoringchanges.h src/plugins/texteditor
% mv deleted/src/plugins/texteditor/textdocument.h src/plugins/texteditor
% mv deleted/src/plugins/texteditor/texteditor_global.h src/plugins/texteditor
```
```
% mkdir src/plugins/vcsbase
% mv deleted/src/plugins/vcsbase/vcsbaseconstants.h ./src/plugins/vcsbase
```

libs directory
----
```
## do not delete following 5 file/directories
#3rdparty
#CMakeLists.txt
#aggregation
#extensionsystem
#utils

rm -rf glsl
rm -rf libs.qbs
rm -rf qlitehtml
rm -rf qmljs
rm -rf sqlite
rm -rf cplusplus
rm -rf languageserverprotocol
rm -rf modelinglib
rm -rf qmldebug
rm -rf qt-breakpad
rm -rf tracing
rm -rf advanceddockingsystem
rm -rf languageutils
rm -rf nanotrace
rm -rf qmleditorwidgets
rm -rf qtcreatorcdbext
```

tools directory
----
```
% mv src/tools deleted/src
% mkdir src/tools
% mv deleted/src/tools/qtcreatorcrashhandler src/tools
```

src/share directory
-------

rm -rf qtcreator
