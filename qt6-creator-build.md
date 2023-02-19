Git
---

git clone https://github.com/qt-creator/qt-creator.git
cd qt-creator.git
git checkout v9.0.1

Build
-----
```bash
% CMAKE_PREFIX_PATH=/opt/Qt6/6.4.0/macos/lib/cmake/ cmake <qt-creator-src-dir>
% make -j8
```

Above command creates "Qt Creator.app" on macOS.

Change IDE output name
----------------------
Change name in ./qt-creator/cmake/QtCreatorIDEBranding.cmake:


Exclude unused modules
------------
1. Exclude tools from src/CMakeLists.txt
2. Exclude files except for following from src/libs/CMakeLists.txt
```
add_subdirectory(aggregation)
add_subdirectory(extensionsystem)
add_subdirectory(utils)
```

3. Exclude lines except for following from src/shared/CMakeLists.txt
```
add_subdirectory(qtsingleapplication)
add_subdirectory(qtlockedfile)
```

4. Exclude all plugins except for coreplugin from src/plugins/CMakeLists.txt
5. Exclude all except following from shared/CMaleLists.txt

tsingleapplication
lockedfile


Some of the header files in deleted project are still referenced from core modules.

```
./src/share/3rdparty
./src/share/qtcreator

./src/plugins/texteditor/blockrange.h
./src/plugins/texteditor/formatter.h
./src/plugins/texteditor/indenter.h
./src/plugins/texteditor/refactoringchanges.h
./src/plugins/texteditor/textdocument.h
./src/plugins/texteditor/texteditor_global.h

./src/plugins/vcsbase/vcsbaseconstants.h
```
