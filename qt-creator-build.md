Git
---

```
git clone https://github.com/qt-creator/qt-creator.git
cd qt-creator
git checkout v8.0.2
```

v8.0.2 seems a last version, which support Qt5

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
Change name in ./qt-creator/cmake/QtCreatorIDEBranding.cmake as following:

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


Exclude unused modules
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
% mv src/shared/pch_files.qbs deleted/src/shared
% mv src/src.qbs deleted/src

% mv qtcreator.pri deleted
% mv qtcreator.qbs deleted
% mv qtcreator_ide_branding.pri deleted
% mv qtcreator_testvars.pri deleted
% mv .tag deleted
```


2. plugin directory
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

script
---
```
#!/bin/bash
deleted=qt-creator.deleted

mkdir -p ${deleted}
mkdir -p ${deleted}/src
mkdir -p ${deleted}/src/shared
mkdir -p ${deleted}/src/libs
mkdir -p ${deleted}/src/plugins

mv tests ${deleted}
mv doc ${deleted}
mv scripts ${deleted}
mv qtcreator.pri ${deleted}
mv qtcreator.qbs ${deleted}
mv qtcreator_ide_branding.pri ${deleted}
mv qtcreator_testvars.pri ${deleted}
mv .tag ${deleted}
mv packaging ${deleted}
mv qbs ${deleted}

mv src/shared/cpaster ${deleted}/src/shared
mv src/shared/designerintegrationv2 ${deleted}/src/shared
mv src/shared/help/ ${deleted}/src/shared
mv src/shared/json ${deleted}/src/shared
mv src/shared/modeltest ${deleted}/src/shared
mv src/shared/proparser ${deleted}/src/shared
mv src/shared/registryaccess ${deleted}/src/shared

echo "----------- src/libs/3rdparty ------------"
mv src/libs/* ${deleted}/src/libs/
mv ${deleted}/src/libs/CMakeLists.txt ./src/libs
mv ${deleted}/src/libs/utils ./src/libs
mv ${deleted}/src/libs/extensionsystem ./src/libs
mv ${deleted}/src/libs/aggregation ./src/libs
mv ${deleted}/src/libs/3rdparty ./src/libs
mv src/libs/3rdparty/cplusplus ${deleted}/src/libs/3rdparty
mv src/libs/3rdparty/json ${deleted}/src/libs/3rdparty
mv src/libs/3rdparty/sqlite ${deleted}/src/libs/3rdparty
mv src/libs/3rdparty/syntax-highlighting ${deleted}/src/libs/3rdparty
mv src/libs/3rdparty/yaml-cpp ${deleted}/src/libs/3rdparty

mv src/shared/pch_files.qbs ${deleted}/src/shared
mv src/src.qbs ${deleted}/src

mv src/plugins/* ${deleted}/src/plugins
mv ${deleted}/src/plugins/CMakeLists.txt src/plugins
mv ${deleted}/src/plugins/coreplugin src/plugins

mkdir -p src/plugins/texteditor
mv ${deleted}/src/plugins/texteditor/blockrange.h src/plugins/texteditor
mv ${deleted}/src/plugins/texteditor/formatter.h src/plugins/texteditor
mv ${deleted}/src/plugins/texteditor/indenter.h src/plugins/texteditor
mv ${deleted}/src/plugins/texteditor/refactoringchanges.h src/plugins/texteditor
mv ${deleted}/src/plugins/texteditor/textdocument.h src/plugins/texteditor
mv ${deleted}/src/plugins/texteditor/texteditor_global.h src/plugins/texteditor

mkdir src/plugins/vcsbase
mv ${deleted}/src/plugins/vcsbase/vcsbaseconstants.h ./src/plugins/vcsbase

echo "----------- src/tools ------------"
mv src/tools ${deleted}/src
mkdir src/tools
mv ${deleted}/src/tools/qtcreatorcrashhandler src/tools
```
