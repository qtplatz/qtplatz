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

#include "pluginview.h"
#include "pluginmanager.h"
#include "pluginspec.h"
#include "plugincollection.h"
#include <utils/itemviews.h>

#include <QDebug>
#include <QDir>
#include <QGridLayout>
#include <QHeaderView>
#include <QPalette>
#include <QTreeWidgetItem>

/*!
    \class ExtensionSystem::PluginView
    \brief The PluginView class implements a widget that shows a list of all
    plugins and their state.

    This class can be embedded for example in a dialog in the application that
    uses the plugin manager.
    The class also provides notifications for interaction with the list.

    \sa ExtensionSystem::PluginDetailsView
    \sa ExtensionSystem::PluginErrorView
*/

/*!
    \fn void PluginView::currentPluginChanged(ExtensionSystem::PluginSpec *spec)
    The current selection in the plugin list has changed to the
    plugin corresponding to \a spec.
*/

/*!
    \fn void PluginView::pluginActivated(ExtensionSystem::PluginSpec *spec)
    The plugin list entry corresponding to \a spec has been activated,
    for example by a double-click.
*/

using namespace ExtensionSystem;

Q_DECLARE_METATYPE(ExtensionSystem::PluginSpec*)
Q_DECLARE_METATYPE(ExtensionSystem::PluginCollection*)

/*!
    Constructs a PluginView that gets the list of plugins from the
    given plugin \a manager with a given \a parent widget.
*/
PluginView::PluginView(QWidget *parent)
    : QWidget(parent),
      m_allowCheckStateUpdate(true),
      C_LOAD(1)
{
    m_categoryWidget = new Utils::TreeWidget(this);
    m_categoryWidget->setAlternatingRowColors(true);
    m_categoryWidget->setIndentation(20);
    m_categoryWidget->setUniformRowHeights(true);
    m_categoryWidget->setSortingEnabled(true);
    m_categoryWidget->setColumnCount(4);
    m_categoryWidget->setColumnWidth(C_LOAD, 40);
    m_categoryWidget->header()->setDefaultSectionSize(120);
    m_categoryWidget->header()->setMinimumSectionSize(35);
    m_categoryWidget->setActivationMode(Utils::DoubleClickActivation);

    QTreeWidgetItem *headerItem = m_categoryWidget->headerItem();
    headerItem->setText(0, tr("Name"));
    headerItem->setText(1, tr("Load"));
    headerItem->setText(2, tr("Version"));
    headerItem->setText(3, tr("Vendor"));

    QGridLayout *gridLayout = new QGridLayout(this);
    gridLayout->setContentsMargins(2, 2, 2, 2);
    gridLayout->addWidget(m_categoryWidget, 1, 0, 1, 1);

    QHeaderView *header = m_categoryWidget->header();
    header->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(2, QHeaderView::ResizeToContents);

    m_okIcon = QIcon(QLatin1String(":/extensionsystem/images/ok.png"));
    m_errorIcon = QIcon(QLatin1String(":/extensionsystem/images/error.png"));
    m_notLoadedIcon = QIcon(QLatin1String(":/extensionsystem/images/notloaded.png"));

    connect(PluginManager::instance(), SIGNAL(pluginsChanged()), this, SLOT(updateList()));
    connect(m_categoryWidget, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
            this, SLOT(selectPlugin(QTreeWidgetItem*)));
    connect(m_categoryWidget, SIGNAL(itemActivated(QTreeWidgetItem*,int)),
            this, SLOT(activatePlugin(QTreeWidgetItem*)));

    updateList();
}

/*!
    \internal
*/
PluginView::~PluginView()
{
}

/*!
    Returns the current selection in the list of plugins.
*/
PluginSpec *PluginView::currentPlugin() const
{
    if (!m_categoryWidget->currentItem())
        return 0;
    if (!m_categoryWidget->currentItem()->data(0, Qt::UserRole).isNull())
        return m_categoryWidget->currentItem()->data(0, Qt::UserRole).value<PluginSpec *>();
    return 0;
}

void PluginView::updateList()
{
    connect(m_categoryWidget, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
            this, SLOT(updatePluginSettings(QTreeWidgetItem*,int)));

    PluginCollection *defaultCollection = 0;
    foreach (PluginCollection *collection, PluginManager::pluginCollections()) {
        if (collection->name().isEmpty() || collection->plugins().isEmpty()) {
            defaultCollection = collection;
            continue;
        }
        // State, name, load, version, vendor.
        QTreeWidgetItem *collectionItem = new QTreeWidgetItem(QStringList()
            << collection->name()
            << QString()    // state
            << QString()    // load
            << QString()    // version
            << QString());  // vendor
        m_items.append(collectionItem);

        Qt::CheckState groupState = Qt::Unchecked;
        int state = parsePluginSpecs(collectionItem, groupState, collection->plugins());

        collectionItem->setIcon(0, iconForState(state));
        collectionItem->setData(C_LOAD, Qt::CheckStateRole, QVariant(groupState));
        collectionItem->setToolTip(C_LOAD, tr("Load on Startup"));
        collectionItem->setData(0, Qt::UserRole, qVariantFromValue(collection));
    }

    QList<PluginSpec *> plugins = defaultCollection ? defaultCollection->plugins() : QList<PluginSpec *>();
    if (!plugins.isEmpty()) {
        // add all non-categorized plugins into utilities. could also be added as root items
        // but that makes the tree ugly.
        QTreeWidgetItem *defaultCollectionItem = new QTreeWidgetItem(QStringList()
            << QString(tr("Utilities"))
            << QString()
            << QString()
            << QString()
            << QString());

        m_items.append(defaultCollectionItem);
        Qt::CheckState groupState = Qt::Unchecked;
        int state = parsePluginSpecs(defaultCollectionItem, groupState, plugins);

        defaultCollectionItem->setIcon(0, iconForState(state));
        defaultCollectionItem->setData(C_LOAD, Qt::CheckStateRole, QVariant(groupState));
        defaultCollectionItem->setToolTip(C_LOAD, tr("Load on Startup"));
        defaultCollectionItem->setData(0, Qt::UserRole, qVariantFromValue(defaultCollection));
    }

    updatePluginDependencies();

    m_categoryWidget->clear();
    if (!m_items.isEmpty()) {
        m_categoryWidget->addTopLevelItems(m_items);
        m_categoryWidget->expandAll();
    }

    m_categoryWidget->sortItems(0, Qt::AscendingOrder);
    if (m_categoryWidget->topLevelItemCount())
        m_categoryWidget->setCurrentItem(m_categoryWidget->topLevelItem(0));
}

int PluginView::parsePluginSpecs(QTreeWidgetItem *parentItem, Qt::CheckState &groupState, QList<PluginSpec*> plugins)
{
    int ret = 0;
    int checkedCount = 0;

    for (int i = 0; i < plugins.length(); ++i) {
        PluginSpec *spec = plugins[i];
        if (spec->hasError())
            ret |= ParsedWithErrors;

        QTreeWidgetItem *pluginItem = new QTreeWidgetItem(QStringList()
            << spec->name()
            << QString()    // load on startup
            << QString::fromLatin1("%1 (%2)").arg(spec->version(), spec->compatVersion())
            << spec->vendor());

        pluginItem->setToolTip(0, QDir::toNativeSeparators(spec->filePath()));
        bool ok = !spec->hasError();
        QIcon icon = ok ? m_okIcon : m_errorIcon;
        if (ok && (spec->state() != PluginSpec::Running))
            icon = m_notLoadedIcon;

        pluginItem->setIcon(0, icon);
        pluginItem->setData(0, Qt::UserRole, qVariantFromValue(spec));

        Qt::CheckState state = Qt::Unchecked;
        if (spec->isEnabledInSettings()) {
            state = Qt::Checked;
            ++checkedCount;
        }

        if (!spec->isAvailableForHostPlatform()) {
            pluginItem->setData(C_LOAD, Qt::CheckStateRole, Qt::Unchecked);
            pluginItem->setFlags(Qt::ItemIsSelectable);
            pluginItem->setToolTip(C_LOAD, tr("Plugin is not available on this platform."));
        } else if (spec->isRequired()){
            pluginItem->setData(C_LOAD, Qt::CheckStateRole, Qt::Checked);
            pluginItem->setFlags(Qt::ItemIsSelectable);
            pluginItem->setToolTip(C_LOAD, tr("Plugin is required."));
        } else {
            pluginItem->setData(C_LOAD, Qt::CheckStateRole, state);
            pluginItem->setToolTip(C_LOAD, tr("Load on startup"));
        }

        m_specToItem.insert(spec, pluginItem);

        if (parentItem)
            parentItem->addChild(pluginItem);
        else
            m_items.append(pluginItem);
    }

    if (checkedCount == 0) {
        groupState = Qt::Unchecked;
        ret |= ParsedNone;
    } else if (checkedCount == plugins.length()) {
        groupState = Qt::Checked;
        ret |= ParsedAll;
    } else {
        groupState = Qt::PartiallyChecked;
        ret = ret | ParsedPartial;
    }
    return ret;
}

QIcon PluginView::iconForState(int state)
{
    if (state & ParsedWithErrors)
        return m_errorIcon;

    if (state & ParsedNone || state & ParsedPartial)
        return m_notLoadedIcon;

    return m_okIcon;
}

void PluginView::selectPlugin(QTreeWidgetItem *current)
{
    if (!current)
        emit currentPluginChanged(0);
    else if (current->data(0, Qt::UserRole).canConvert<PluginSpec*>())
        emit currentPluginChanged(current->data(0, Qt::UserRole).value<PluginSpec *>());
    else
        emit currentPluginChanged(0);

}

void PluginView::activatePlugin(QTreeWidgetItem *item)
{
    if (item->data(0, Qt::UserRole).canConvert<PluginSpec*>()) {
        emit pluginActivated(item->data(0, Qt::UserRole).value<PluginSpec *>());
    } else
        emit pluginActivated(0);
}

void PluginView::updatePluginSettings(QTreeWidgetItem *item, int column)
{
    if (!m_allowCheckStateUpdate)
        return;

    m_allowCheckStateUpdate = false;

    bool loadOnStartup = item->data(C_LOAD, Qt::CheckStateRole).toBool();

    if (item->data(0, Qt::UserRole).canConvert<PluginSpec*>()) {
        PluginSpec *spec = item->data(0, Qt::UserRole).value<PluginSpec *>();

        if (column == C_LOAD) {

            spec->setEnabled(loadOnStartup);
            updatePluginDependencies();

            if (item->parent()) {
                PluginCollection *collection = item->parent()->data(0, Qt::UserRole).value<PluginCollection *>();
                Qt::CheckState state = Qt::PartiallyChecked;
                int loadCount = 0;
                for (int i = 0; i < collection->plugins().length(); ++i) {
                    if (collection->plugins().at(i)->isEnabledInSettings())
                        ++loadCount;
                }
                if (loadCount == collection->plugins().length())
                    state = Qt::Checked;
                else if (loadCount == 0)
                    state = Qt::Unchecked;

                item->parent()->setData(C_LOAD, Qt::CheckStateRole, state);
            }

            emit pluginSettingsChanged(spec);
        }

    } else {
        PluginCollection *collection = item->data(0, Qt::UserRole).value<PluginCollection *>();
        for (int i = 0; i < collection->plugins().length(); ++i) {
            PluginSpec *spec = collection->plugins().at(i);
            QTreeWidgetItem *child = m_specToItem.value(spec);

            if (spec->isAvailableForHostPlatform() && !spec->isRequired()) {
                spec->setEnabled(loadOnStartup);
                Qt::CheckState state = (loadOnStartup ? Qt::Checked : Qt::Unchecked);
                child->setData(C_LOAD, Qt::CheckStateRole, state);
            }
        }
        updatePluginDependencies();
        if (!collection->plugins().isEmpty())
            emit pluginSettingsChanged(collection->plugins().first());
    }

    m_allowCheckStateUpdate = true;
}

void PluginView::updatePluginDependencies()
{
    foreach (PluginSpec *spec, PluginManager::loadQueue()) {
        bool disableIndirectly = false;
        if (spec->isRequired())
            continue;

        QHashIterator<PluginDependency, PluginSpec *> it(spec->dependencySpecs());
        while (it.hasNext()) {
            it.next();
            if (it.key().type == PluginDependency::Optional)
                continue;
            PluginSpec *depSpec = it.value();
            if (!depSpec->isEnabledInSettings() || depSpec->isDisabledIndirectly()) {
                disableIndirectly = true;
                break;
            }
        }
        QTreeWidgetItem *childItem = m_specToItem.value(spec);
        childItem->setDisabled(disableIndirectly || !spec->isAvailableForHostPlatform());

        if (disableIndirectly == spec->isDisabledIndirectly())
            continue;
        spec->setDisabledIndirectly(disableIndirectly);

        if (childItem->parent() && !childItem->parent()->isExpanded())
            childItem->parent()->setExpanded(true);
    }
}
