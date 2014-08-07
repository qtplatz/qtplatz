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

#ifndef IDOCUMENT_H
#define IDOCUMENT_H

#include "core_global.h"
#include "id.h"

#include <QObject>

namespace Core {

class MimeType;
class InfoBar;

class CORE_EXPORT IDocument : public QObject
{
    Q_OBJECT

public:
    // This enum must match the indexes of the reloadBehavior widget
    // in generalsettings.ui
    enum ReloadSetting {
        AlwaysAsk = 0,
        ReloadUnmodified = 1,
        IgnoreAll = 2
    };

    enum ChangeTrigger {
        TriggerInternal,
        TriggerExternal
    };

    enum ChangeType {
        TypeContents,
        TypePermissions,
        TypeRemoved
    };

    enum ReloadBehavior {
        BehaviorAsk,
        BehaviorSilent
    };

    enum ReloadFlag {
        FlagReload,
        FlagIgnore
    };

    IDocument(QObject *parent = 0);
    virtual ~IDocument();

    void setId(Core::Id id);
    Core::Id id() const;

    virtual bool save(QString *errorString, const QString &fileName = QString(), bool autoSave = false) = 0;
    virtual bool setContents(const QByteArray &contents);

    QString filePath() const { return m_filePath; }
    virtual void setFilePath(const QString &filePath);
    QString displayName() const;
    void setDisplayName(const QString &name);

    virtual bool isFileReadOnly() const;
    bool isTemporary() const;
    void setTemporary(bool temporary);

    virtual QString defaultPath() const = 0;
    virtual QString suggestedFileName() const = 0;

    QString mimeType() const { return m_mimeType; }
    void setMimeType(const QString &mimeType);

    virtual bool shouldAutoSave() const;
    virtual bool isModified() const = 0;
    virtual bool isSaveAsAllowed() const = 0;

    virtual ReloadBehavior reloadBehavior(ChangeTrigger state, ChangeType type) const;
    virtual bool reload(QString *errorString, ReloadFlag flag, ChangeType type) = 0;

    virtual void checkPermissions();

    bool autoSave(QString *errorString, const QString &filePath);
    void setRestoredFrom(const QString &name);
    void removeAutoSaveFile();

    bool hasWriteWarning() const { return m_hasWriteWarning; }
    void setWriteWarning(bool has) { m_hasWriteWarning = has; }

    InfoBar *infoBar();

signals:
    void changed();
    void mimeTypeChanged();

    void aboutToReload();
    void reloadFinished(bool success);

    void filePathChanged(const QString &oldName, const QString &newName);

private:
    Id m_id;
    QString m_mimeType;
    QString m_filePath;
    QString m_displayName;
    bool m_temporary;
    QString m_autoSaveName;
    InfoBar *m_infoBar;
    bool m_hasWriteWarning;
    bool m_restored;
};

} // namespace Core

#endif // IDOCUMENT_H
