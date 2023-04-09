/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtCore module of the Qt Toolkit.
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

#include "mimeutils.h"

#include "mimedatabase.h"
#include "mimedatabase_p.h"
#include "mimemagicrule_p.h"
#include "mimeprovider_p.h"

#include "filepath.h"

using namespace Utils;
using namespace Utils::Internal;

void Utils::addMimeTypes(const QString &fileName, const QByteArray &data)
{
    auto d = MimeDatabasePrivate::instance();
    QMutexLocker locker(&d->mutex);

    if (d->m_startupPhase >= MimeDatabase::PluginsDelayedInitializing)
        qWarning("Adding items from %s to MimeDatabase after initialization time",
                 qPrintable(fileName));

    auto xmlProvider = static_cast<MimeXMLProvider *>(d->provider());
    xmlProvider->addData(fileName, data);
}

QMap<int, QList<MimeMagicRule>> Utils::magicRulesForMimeType(const MimeType &mimeType)
{
    auto d = MimeDatabasePrivate::instance();
    QMutexLocker locker(&d->mutex);
    return d->provider()->magicRulesForMimeType(mimeType);
}

void Utils::setGlobPatternsForMimeType(const MimeType &mimeType, const QStringList &patterns)
{
    auto d = MimeDatabasePrivate::instance();
    QMutexLocker locker(&d->mutex);
    d->provider()->setGlobPatternsForMimeType(mimeType, patterns);
}

void Utils::setMagicRulesForMimeType(const MimeType &mimeType,
                                     const QMap<int, QList<MimeMagicRule>> &rules)
{
    auto d = MimeDatabasePrivate::instance();
    QMutexLocker locker(&d->mutex);
    d->provider()->setMagicRulesForMimeType(mimeType, rules);
}

void Utils::setMimeStartupPhase(MimeStartupPhase phase)
{
    auto d = MimeDatabasePrivate::instance();
    QMutexLocker locker(&d->mutex);
    if (int(phase) != d->m_startupPhase + 1)
        qWarning("Unexpected jump in MimedDatabase lifetime from %d to %d",
                 d->m_startupPhase,
                 int(phase));
    d->m_startupPhase = int(phase);
}

MimeType Utils::mimeTypeForName(const QString &nameOrAlias)
{
    MimeDatabase mdb;
    return mdb.mimeTypeForName(nameOrAlias);
}

MimeType Utils::mimeTypeForFile(const QString &fileName, MimeMatchMode mode)
{
    MimeDatabase mdb;
    return mdb.mimeTypeForFile(fileName, MimeDatabase::MatchMode(mode));
}

MimeType Utils::mimeTypeForFile(const QFileInfo &fileInfo, MimeMatchMode mode)
{
    MimeDatabase mdb;
    return mdb.mimeTypeForFile(fileInfo, MimeDatabase::MatchMode(mode));
}

MimeType Utils::mimeTypeForFile(const FilePath &filePath, MimeMatchMode mode)
{
    MimeDatabase mdb;
    if (filePath.needsDevice())
        return mdb.mimeTypeForUrl(filePath.toUrl());
    return mdb.mimeTypeForFile(filePath.toString(), MimeDatabase::MatchMode(mode));
}

QList<MimeType> Utils::mimeTypesForFileName(const QString &fileName)
{
    MimeDatabase mdb;
    return mdb.mimeTypesForFileName(fileName);
}

MimeType Utils::mimeTypeForData(const QByteArray &data)
{
    MimeDatabase mdb;
    return mdb.mimeTypeForData(data);
}

QList<MimeType> Utils::allMimeTypes()
{
    MimeDatabase mdb;
    return mdb.allMimeTypes();
}
