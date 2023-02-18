Git
---

git clone https://github.com/qt-creator/qt-creator.git

Build
-----
```bash
% CMAKE_PREFIX_PATH=/opt/Qt6/6.4.0/macos/lib/cmake/ cmake <qt-creator-src-dir>
% make -j8
```

Above command creates "Qt Creator.app" on macOS.

Exclude unused modules
------------
1. Exclude tools from src/CMakeLists.txt
2. Exclude files except following from src/libs/CMakeLists.txt
```
add_subdirectory(aggregation)
add_subdirectory(extensionsystem)
add_subdirectory(utils)
```

2. Exclude all plugins except for coreplugin from src/plugins/CMakeLists.txt

Exclude from src/shared/CMakeLists.txt
```
#add_subdirectory(designerintegrationv2)
#add_subdirectory(proparser)
add_subdirectory(qtsingleapplication)
add_subdirectory(qtlockedfile)
#add_subdirectory(help)
#add_subdirectory(registryaccess)
```

Disable plugins marked as level 6,7, and 8 (coreplugin) by adding if (FALSE) .. endif() to src/plugins/CMakeFiles.txt.
It has been built.

Second attempt
------------

Disable level 5, causes an error and stop build.
```
[ 89%] Linking CXX executable "../../../Qt Creator.app/Contents/Resources/libexec/buildoutputparser"
ld: library not found for -lQmakeProjectManager
```

try disable tools in src/CMakeLists.txt -- OK, it went through build process.
Exclude level 1 through 8 -- OK, it went through build process.

--------------
Exclude plugins except for coreplugin
--------------

In summary, remove
1. tools in src/CMakeLists.txt, and
2. remove all plugins except for coreplugin from src/plugins/CMakeLists.txt

hat builds minium qt-creator app.

More exclusions
----------------

Exclude share sub-directory from src/CMakeLists.txt

remove follwing from src/libs/CMakeLists.txt

```
#add_subdirectory(cplusplus)
#add_subdirectory(modelinglib)
#add_subdirectory(nanotrace)
#add_subdirectory(qmljs)
#add_subdirectory(qmldebug)
#add_subdirectory(qmleditorwidgets)
#add_subdirectory(glsl)
#add_subdirectory(languageserverprotocol)
#add_subdirectory(sqlite)
#add_subdirectory(tracing)
#add_subdirectory(qmlpuppetcommunication)

#add_subdirectory(qtcreatorcdbext)
```

Delete all except add_subdirectory(qtsingleapplication), lockedfile from shared/CMaleLists.txt
