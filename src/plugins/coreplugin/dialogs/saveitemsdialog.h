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

#ifndef SAVEITEMSDIALOG_H
#define SAVEITEMSDIALOG_H

#include <QList>
#include <QDialog>

#include "ui_saveitemsdialog.h"

QT_BEGIN_NAMESPACE
class QCheckBox;
QT_END_NAMESPACE

namespace Core {

class IDocument;
class EditorManager;

namespace Internal {

class MainWindow;

class SaveItemsDialog : public QDialog
{
    Q_OBJECT

public:
    SaveItemsDialog(QWidget *parent,
        QList<Core::IDocument *> items);

    void setMessage(const QString &msg);
    void setAlwaysSaveMessage(const QString &msg);
    bool alwaysSaveChecked();
    QList<Core::IDocument *> itemsToSave() const;

private slots:
    void collectItemsToSave();
    void discardAll();
    void updateSaveButton();

private:
    void adjustButtonWidths();

    Ui::SaveItemsDialog m_ui;
    QList<Core::IDocument*> m_itemsToSave;
};

} // namespace Internal
} // namespace Core

#endif // SAVEITEMSDIALOG_H
