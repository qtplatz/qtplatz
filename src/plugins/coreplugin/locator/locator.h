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

#ifndef LOCATORPLUGIN_H
#define LOCATORPLUGIN_H

#include "ilocatorfilter.h"
#include "directoryfilter.h"
#include "executefilter.h"
#include "locatorconstants.h"

#include <extensionsystem/iplugin.h>
#include <coreplugin/actionmanager/command.h>

#include <QTimer>
#include <QFutureWatcher>

namespace Core {
namespace Internal {

class CorePlugin;
class LocatorWidget;
class OpenDocumentsFilter;
class FileSystemFilter;
class SettingsPage;

class Locator : public QObject
{
    Q_OBJECT

public:
    Locator();
    ~Locator();

    void initialize(CorePlugin *corePlugin, const QStringList &arguments, QString *errorMessage);
    void extensionsInitialized();
    bool delayedInitialize();

    QList<ILocatorFilter *> filters();
    QList<ILocatorFilter *> customFilters();
    void setFilters(QList<ILocatorFilter *> f);
    void setCustomFilters(QList<ILocatorFilter *> f);
    int refreshInterval();
    void setRefreshInterval(int interval);

public slots:
    void refresh(QList<ILocatorFilter *> filters = QList<ILocatorFilter *>());
    void saveSettings();
    void openLocator();

private slots:
    void updatePlaceholderText(Core::Command *command = 0);

private:
    void loadSettings();

    template <typename S>
    void loadSettingsHelper(S *settings);

    LocatorWidget *m_locatorWidget;
    SettingsPage *m_settingsPage;

    bool m_settingsInitialized;
    QList<ILocatorFilter *> m_filters;
    QList<ILocatorFilter *> m_customFilters;
    int m_refreshInterval;
    QTimer m_refreshTimer;
    OpenDocumentsFilter *m_openDocumentsFilter;
    FileSystemFilter *m_fileSystemFilter;
    ExecuteFilter *m_executeFilter;
    CorePlugin *m_corePlugin;
};

template <typename S>
void Locator::loadSettingsHelper(S *settings)
{
    settings->beginGroup(QLatin1String("QuickOpen"));
    m_refreshTimer.setInterval(settings->value(QLatin1String("RefreshInterval"), 60).toInt() * 60000);

    foreach (ILocatorFilter *filter, m_filters) {
        if (settings->contains(filter->id().toString())) {
            const QByteArray state = settings->value(filter->id().toString()).toByteArray();
            if (!state.isEmpty())
                filter->restoreState(state);
        }
    }
    settings->beginGroup(QLatin1String("CustomFilters"));
    QList<ILocatorFilter *> customFilters;
    const QStringList keys = settings->childKeys();
    int count = 0;
    Core::Id baseId(Constants::CUSTOM_FILTER_BASEID);
    foreach (const QString &key, keys) {
        ILocatorFilter *filter = new DirectoryFilter(baseId.withSuffix(++count));
        filter->restoreState(settings->value(key).toByteArray());
        m_filters.append(filter);
        customFilters.append(filter);
    }
    setCustomFilters(customFilters);
    settings->endGroup();
    settings->endGroup();
}

} // namespace Internal
} // namespace Core

#endif // LOCATORPLUGIN_H
