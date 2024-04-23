/****************************************************************************
**
** Copyright (C) 2019 Denis Shienkov <denis.shienkov@gmail.com>
** Contact: http://www.qt.io/licensing
**
** This file is part of Qbs.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms and
** conditions see http://www.qt.io/terms-conditions. For further information
** use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file.  Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, The Qt Company gives you certain additional
** rights.  These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

import qbs.File
import "../../../modules/cpp/sdcc.js" as SDCC

PathProbe {
    // Inputs
    property string compilerFilePath
    property stringList enableDefinesByLanguage
    property string preferredArchitecture

    // Outputs
    property string architecture
    property string endianness
    property int versionMajor
    property int versionMinor
    property int versionPatch
    property stringList includePaths
    property var compilerDefinesByLanguage

    configure: {
        compilerDefinesByLanguage = {};

        if (!File.exists(compilerFilePath)) {
            found = false;
            return;
        }

        var languages = enableDefinesByLanguage;
        if (!languages || languages.length === 0)
            languages = ["c"];

        // SDCC compiler support only the C-language.
        if (!languages.contains("c")) {
            found = false;
            return;
        }

        var macros = SDCC.dumpMacros(compilerFilePath, preferredArchitecture);
        if (!macros) {
            found = false;
            return;
        }

        compilerDefinesByLanguage["c"] = macros;

        architecture = SDCC.guessArchitecture(macros);
        endianness = SDCC.guessEndianness(macros);

        var defaultPaths = SDCC.dumpDefaultPaths(compilerFilePath, architecture);
        includePaths = defaultPaths.includePaths;

        var version = SDCC.guessVersion(macros);
        if (version) {
            versionMajor = version.major;
            versionMinor = version.minor;
            versionPatch = version.patch;
            found = !!architecture && !!endianness;
        }
   }
}
