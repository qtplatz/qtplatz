/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qbs.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QBS_HOSTOSINFO_H
#define QBS_HOSTOSINFO_H

#include "qbs_export.h"
#include "stlutils.h"
#include "version.h"

#include <QtCore/qglobal.h>
#include <QtCore/qmap.h>
#include <QtCore/qsettings.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>

#if defined(Q_OS_WIN)
#define QBS_HOST_EXE_SUFFIX ".exe"
#define QBS_HOST_DYNAMICLIB_PREFIX ""
#define QBS_HOST_DYNAMICLIB_SUFFIX ".dll"
#elif defined(Q_OS_DARWIN)
#define QBS_HOST_EXE_SUFFIX ""
#define QBS_HOST_DYNAMICLIB_PREFIX "lib"
#define QBS_HOST_DYNAMICLIB_SUFFIX ".dylib"
#else
#define QBS_HOST_EXE_SUFFIX ""
#define QBS_HOST_DYNAMICLIB_PREFIX "lib"
#define QBS_HOST_DYNAMICLIB_SUFFIX ".so"
#endif // Q_OS_WIN

namespace qbs {
namespace Internal {

class HostOsInfo
{
public:
    // Add more as needed.
    enum HostOs { HostOsWindows, HostOsLinux, HostOsMacos, HostOsOtherUnix, HostOsOther };

    static inline QString hostOSIdentifier();
    static inline QString hostOSArchitecture();
    static inline std::vector<QString> hostOSIdentifiers();
    static inline std::vector<QString> canonicalOSIdentifiers(const QString &os);
    static inline HostOs hostOs();

    static inline Version hostOsVersion() {
        Version v;
        if (HostOsInfo::isWindowsHost()) {
            QSettings settings(QStringLiteral("HKEY_LOCAL_MACHINE\\Software\\"
                                                   "Microsoft\\Windows NT\\CurrentVersion"),
                               QSettings::NativeFormat);
            v = v.fromString(settings.value(QStringLiteral("CurrentVersion")).toString() +
                             QLatin1Char('.') +
                             settings.value(QStringLiteral("CurrentBuildNumber")).toString());
            Q_ASSERT(v.isValid());
        } else if (HostOsInfo::isMacosHost()) {
            QSettings settings(QStringLiteral("/System/Library/CoreServices/SystemVersion.plist"),
                               QSettings::NativeFormat);
            v = v.fromString(settings.value(QStringLiteral("ProductVersion")).toString());
            Q_ASSERT(v.isValid());
        }
        return v;
    }

    static QString hostOsBuildVersion() {
        QString v;
        if (HostOsInfo::isWindowsHost()) {
            QSettings settings(QStringLiteral("HKEY_LOCAL_MACHINE\\Software\\"
                                                   "Microsoft\\Windows NT\\CurrentVersion"),
                               QSettings::NativeFormat);
            v = settings.value(QStringLiteral("CurrentBuildNumber")).toString();
        } else if (HostOsInfo::isMacosHost()) {
            QSettings serverSettings(QStringLiteral(
                                         "/System/Library/CoreServices/ServerVersion.plist"),
                               QSettings::NativeFormat);
            v = serverSettings.value(QStringLiteral("ProductBuildVersion")).toString();
            if (v.isNull()) {
                QSettings systemSettings(QStringLiteral(
                                             "/System/Library/CoreServices/SystemVersion.plist"),
                                   QSettings::NativeFormat);
                v = systemSettings.value(QStringLiteral("ProductBuildVersion")).toString();
            }
        }
        return v;
    }

    static bool isWindowsHost() { return hostOs() == HostOsWindows; }
    static bool isLinuxHost() { return hostOs() == HostOsLinux; }
    static bool isMacosHost() { return hostOs() == HostOsMacos; }
    static inline bool isAnyUnixHost();
    static inline QString rfc1034Identifier(const QString &str);

    static QString appendExecutableSuffix(const QString &executable)
    {
        QString finalName = executable;
        if (isWindowsHost())
            finalName += QLatin1String(QBS_HOST_EXE_SUFFIX);
        return finalName;
    }

    static QString stripExecutableSuffix(const QString &executable)
    {
        constexpr QLatin1String suffix(QBS_HOST_EXE_SUFFIX, sizeof(QBS_HOST_EXE_SUFFIX) - 1);
        return !suffix.isEmpty() && executable.endsWith(suffix)
            ? executable.chopped(suffix.size()) : executable;
    }

    static QString dynamicLibraryName(const QString &libraryBaseName)
    {
        return QLatin1String(QBS_HOST_DYNAMICLIB_PREFIX) + libraryBaseName
                + QLatin1String(QBS_HOST_DYNAMICLIB_SUFFIX);
    }

    static Qt::CaseSensitivity fileNameCaseSensitivity()
    {
        return isWindowsHost() ? Qt::CaseInsensitive: Qt::CaseSensitive;
    }

    static QString libraryPathEnvironmentVariable()
    {
        if (isWindowsHost())
            return QStringLiteral("PATH");
        if (isMacosHost())
            return QStringLiteral("DYLD_LIBRARY_PATH");
        return QStringLiteral("LD_LIBRARY_PATH");
    }

    static QChar pathListSeparator(HostOsInfo::HostOs hostOs = HostOsInfo::hostOs())
    {
        return hostOs == HostOsWindows ? QLatin1Char(';') : QLatin1Char(':');
    }

    static QChar pathSeparator(HostOsInfo::HostOs hostOs = HostOsInfo::hostOs())
    {
        return hostOs == HostOsWindows ? QLatin1Char('\\') : QLatin1Char('/');
    }

    static Qt::KeyboardModifier controlModifier()
    {
        return isMacosHost() ? Qt::MetaModifier : Qt::ControlModifier;
    }
};

QString HostOsInfo::hostOSIdentifier()
{
#if defined(__APPLE__)
    return QStringLiteral("macos");
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    return QStringLiteral("windows");
#elif defined(_AIX)
    return QStringLiteral("aix");
#elif defined(hpux) || defined(__hpux)
    return QStringLiteral("hpux");
#elif defined(__sun) || defined(sun)
    return QStringLiteral("solaris");
#elif defined(__linux__) || defined(__linux)
    return QStringLiteral("linux");
#elif defined(__FreeBSD__) || defined(__DragonFly__) || defined(__FreeBSD_kernel__)
    return QStringLiteral("freebsd");
#elif defined(__NetBSD__)
    return QStringLiteral("netbsd");
#elif defined(__OpenBSD__)
    return QStringLiteral("openbsd");
#elif defined(__GNU__)
    return QStringLiteral("hurd");
#elif defined(__HAIKU__)
    return QStringLiteral("haiku");
#else
    #warning "Qbs has not been ported to this OS - see http://qbs.io/"
    return {};
#endif
}

QString HostOsInfo::hostOSArchitecture()
{
    const auto cpuArch = QSysInfo::currentCpuArchitecture();
    if (cpuArch == QLatin1String("i386"))
        return QStringLiteral("x86");
    return cpuArch;
}

std::vector<QString> HostOsInfo::hostOSIdentifiers()
{
    return canonicalOSIdentifiers(hostOSIdentifier());
}

std::vector<QString> HostOsInfo::canonicalOSIdentifiers(const QString &name)
{
    std::vector<QString> list { name };
    if (contains({u"ios-simulator"}, name))
        list << canonicalOSIdentifiers(QStringLiteral("ios"));
    if (contains({u"tvos-simulator"}, name))
        list << canonicalOSIdentifiers(QStringLiteral("tvos"));
    if (contains({u"watchos-simulator"}, name))
        list << canonicalOSIdentifiers(QStringLiteral("watchos"));
    if (contains({u"macos", u"ios", u"tvos", u"watchos"}, name))
        list << canonicalOSIdentifiers(QStringLiteral("darwin"));
    if (contains({u"darwin", u"freebsd", u"netbsd", u"openbsd"}, name))
        list << canonicalOSIdentifiers(QStringLiteral("bsd"));
    if (contains({u"android"}, name))
        list << canonicalOSIdentifiers(QStringLiteral("linux"));

    // Note: recognized non-Unix platforms include: windows, haiku, vxworks
    if (contains({u"bsd", u"aix", u"hpux", u"solaris", u"linux", u"hurd", u"qnx", u"integrity"}, name))
        list << canonicalOSIdentifiers(QStringLiteral("unix"));

    return list;
}

HostOsInfo::HostOs HostOsInfo::hostOs()
{
#if defined(Q_OS_WIN)
    return HostOsWindows;
#elif defined(Q_OS_LINUX)
    return HostOsLinux;
#elif defined(Q_OS_DARWIN)
    return HostOsMacos;
#elif defined(Q_OS_UNIX)
    return HostOsOtherUnix;
#else
    return HostOsOther;
#endif
}

bool HostOsInfo::isAnyUnixHost()
{
#ifdef Q_OS_UNIX
    return true;
#else
    return false;
#endif
}

QString HostOsInfo::rfc1034Identifier(const QString &str)
{
    QString s = str;
    for (QChar &ch : s) {
        const char c = ch.toLatin1();

        const bool okChar = (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z')
                || (c >= 'a' && c <= 'z') || c == '-' || c == '.';
        if (!okChar)
            ch = QChar::fromLatin1('-');
    }
    return s;
}

} // namespace Internal
} // namespace qbs

#endif // QBS_HOSTOSINFO_H
