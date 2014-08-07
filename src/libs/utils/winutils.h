/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#ifndef WINUTILS_H
#define WINUTILS_H

#include "utils_global.h"

namespace Utils {

// Helper to format a Windows error message, taking the
// code as returned by the GetLastError()-API.
QTCREATOR_UTILS_EXPORT QString winErrorMessage(unsigned long error);

// Determine a DLL version
enum WinDLLVersionType { WinDLLFileVersion, WinDLLProductVersion };
QTCREATOR_UTILS_EXPORT QString winGetDLLVersion(WinDLLVersionType t,
                                                const QString &name,
                                                QString *errorMessage);

QTCREATOR_UTILS_EXPORT bool is64BitWindowsSystem();

// Check for a 64bit binary.
QTCREATOR_UTILS_EXPORT bool is64BitWindowsBinary(const QString &binary);

//
// RAII class to temporarily prevent windows crash messages from popping up using the
// application-global (!) error mode.
//
// Useful primarily for QProcess launching, since the setting will be inherited.
//
class QTCREATOR_UTILS_EXPORT WindowsCrashDialogBlocker {
public:
    WindowsCrashDialogBlocker();
    ~WindowsCrashDialogBlocker();
#ifdef Q_OS_WIN
private:
    const unsigned int silenceErrorMode;
    const unsigned int originalErrorMode;
#endif // Q_OS_WIN
};

} // namespace Utils

#endif // WINUTILS_H
