// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "unixutils.h"

#include "filepath.h"
#include "qtcsettings.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QSettings>

using namespace Utils;

QString UnixUtils::defaultFileBrowser()
{
    return QLatin1String("xdg-open %d");
}

QString UnixUtils::fileBrowser(const QSettings *settings)
{
    const QString dflt = defaultFileBrowser();
    if (!settings)
        return dflt;
    return settings->value(QLatin1String("General/FileBrowser"), dflt).toString();
}

void UnixUtils::setFileBrowser(QSettings *settings, const QString &term)
{
    QtcSettings::setValueWithDefault(settings, "General/FileBrowser", term, defaultFileBrowser());
}


QString UnixUtils::fileBrowserHelpText()
{
    QString help = QCoreApplication::translate("Utils::UnixTools",
            "<table border=1 cellspacing=0 cellpadding=3>"
            "<tr><th>Variable</th><th>Expands to</th></tr>"
            "<tr><td>%d</td><td>directory of current file</td></tr>"
            "<tr><td>%f</td><td>file name (with full path)</td></tr>"
            "<tr><td>%n</td><td>file name (without path)</td></tr>"
            "<tr><td>%%</td><td>%</td></tr>"
            "</table>");
    return help;
}

QString UnixUtils::substituteFileBrowserParameters(const QString &pre, const QString &file)
{
    QString cmd;
    for (int i = 0; i < pre.size(); ++i) {
        QChar c = pre.at(i);
        if (c == QLatin1Char('%') && i < pre.size()-1) {
            c = pre.at(++i);
            QString s;
            if (c == QLatin1Char('d')) {
                s = QLatin1Char('"') + QFileInfo(file).path() + QLatin1Char('"');
            } else if (c == QLatin1Char('f')) {
                s = QLatin1Char('"') + file + QLatin1Char('"');
            } else if (c == QLatin1Char('n')) {
                s = QLatin1Char('"') + FilePath::fromString(file).fileName() + QLatin1Char('"');
            } else if (c == QLatin1Char('%')) {
                s = c;
            } else {
                s = QLatin1Char('%');
                s += c;
            }
            cmd += s;
            continue;

        }
        cmd += c;
    }

    return cmd;
}
