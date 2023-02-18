// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR LGPL-3.0

#pragma once

#include "mimetype.h"

QT_BEGIN_NAMESPACE
class QFileInfo;
class QIODevice;
class QUrl;
QT_END_NAMESPACE

namespace Utils {

class MimeDatabase
{
    Q_DISABLE_COPY(MimeDatabase)

public:
    MimeDatabase();
    ~MimeDatabase();

    MimeType mimeTypeForName(const QString &nameOrAlias) const;

    enum MatchMode { MatchDefault = 0x0, MatchExtension = 0x1, MatchContent = 0x2 };

    MimeType mimeTypeForFile(const QString &fileName, MatchMode mode = MatchDefault) const;
    MimeType mimeTypeForFile(const QFileInfo &fileInfo, MatchMode mode = MatchDefault) const;
    QList<MimeType> mimeTypesForFileName(const QString &fileName) const;

    MimeType mimeTypeForData(const QByteArray &data) const;
    MimeType mimeTypeForData(QIODevice *device) const;

    MimeType mimeTypeForUrl(const QUrl &url) const;
    MimeType mimeTypeForFileNameAndData(const QString &fileName, QIODevice *device) const;
    MimeType mimeTypeForFileNameAndData(const QString &fileName, const QByteArray &data) const;

    QString suffixForFileName(const QString &fileName) const;

    QList<MimeType> allMimeTypes() const;

    // For debugging purposes.
    enum StartupPhase {
        BeforeInitialize,
        PluginsLoading,
        PluginsInitializing,        // Register up to here.
        PluginsDelayedInitializing, // Use from here on.
        UpAndRunning
    };
    static void setStartupPhase(StartupPhase);

private:
    Internal::MimeDatabasePrivate *d;
};

} // Utils
