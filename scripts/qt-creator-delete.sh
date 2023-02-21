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
