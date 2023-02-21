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

#ifndef FINDPLUGIN_H
#define FINDPLUGIN_H

#include "textfindconstants.h"

#include <extensionsystem/iplugin.h>

QT_BEGIN_NAMESPACE
class QStringListModel;
QT_END_NAMESPACE

namespace Core {
class IFindFilter;
class FindPluginPrivate;

namespace Internal {
class CorePlugin;
class FindToolBar;
class CurrentDocumentFind;
} // namespace Internal

class CORE_EXPORT FindPlugin : public QObject
{
    Q_OBJECT

public:
    FindPlugin();
    virtual ~FindPlugin();

    static FindPlugin *instance();

    enum FindDirection {
        FindForwardDirection,
        FindBackwardDirection
    };

    Core::FindFlags findFlags() const;
    bool hasFindFlag(Core::FindFlag flag);
    void updateFindCompletion(const QString &text);
    void updateReplaceCompletion(const QString &text);
    QStringListModel *findCompletionModel() const;
    QStringListModel *replaceCompletionModel() const;
    void setUseFakeVim(bool on);
    void openFindToolBar(FindDirection direction);
    void openFindDialog(IFindFilter *filter);

    void initialize(const QStringList &, QString *);
    void extensionsInitialized();
    void aboutToShutdown();

public slots:
    void setCaseSensitive(bool sensitive);
    void setWholeWord(bool wholeOnly);
    void setBackward(bool backward);
    void setRegularExpression(bool regExp);
    void setPreserveCase(bool preserveCase);

signals:
    void findFlagsChanged();

private slots:
    void filterChanged();
    void openFindFilter();
    void writeSettings();

private:
    void setFindFlag(Core::FindFlag flag, bool enabled);
    void updateCompletion(const QString &text, QStringList &completions, QStringListModel *model);
    void setupMenu();
    void setupFilterMenuItems();
    void readSettings();

    //variables
    FindPluginPrivate *d;
};

} // namespace Core

#endif // FINDPLUGIN_H
