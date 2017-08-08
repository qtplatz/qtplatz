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

#include "actionmanager.h"
#include "actionmanager_p.h"
#include "actioncontainer_p.h"
#include "command_p.h"
#include <coreplugin/id.h>
#include <coreplugin/mainwindow.h>

#include <utils/qtcassert.h>

#include <QDebug>
#include <QSettings>
#include <QLabel>
#include <QMenu>
#include <QAction>
#include <QShortcut>
#include <QMenuBar>

namespace {
    enum { warnAboutFindFailures = 0 };
}

static const char kKeyboardSettingsKey[] = "KeyboardShortcuts";
static const char kKeyboardSettingsTransferredKey[] = "OldSettingsTransferred";

using namespace Core;
using namespace Core::Internal;

/*!
    \class Core::ActionManager
    \mainclass
    \inmodule Qt Creator

    \brief The ActionManager class is responsible for registration of menus and
    menu items and keyboard shortcuts.

    The ActionManager is the central bookkeeper of actions and their shortcuts and layout.
    It is a singleton containing mostly static functions. If you need access to the instance,
    e.g. for connecting to signals, is its ActionManager::instance() function.

    The main reasons for the need of this class is to provide a central place where the user
    can specify all his keyboard shortcuts, and to provide a solution for actions that should
    behave differently in different contexts (like the copy/replace/undo/redo actions).

    \section1 Contexts

    All actions that are registered with the same Id (but different context lists)
    are considered to be overloads of the same command, represented by an instance
    of the Command class.
    Exactly only one of the registered actions with the same ID is active at any time.
    Which action this is, is defined by the context list that the actions were registered
    with:

    If the current focus widget was registered via \l{ICore::addContextObject()},
    all the contexts returned by its IContext object are active. In addition all
    contexts set via \l{ICore::addAdditionalContext()} are active as well. If one
    of the actions was registered for one of these active contexts, it is the one
    active action, and receives \c triggered and \c toggled signals. Also the
    appearance of the visible action for this ID might be adapted to this
    active action (depending on the settings of the corresponding \l{Command} object).

    The action that is visible to the user is the one returned by Command::action().
    If you provide yourself a user visible representation of your action you need
    to use Command::action() for this.
    When this action is invoked by the user,
    the signal is forwarded to the registered action that is valid for the current context.

    \section1 Registering Actions

    To register a globally active action "My Action"
    put the following in your plugin's IPlugin::initialize function:
    \code
        QAction *myAction = new QAction(tr("My Action"), this);
        Core::Command *cmd = Core::ActionManager::registerAction(myAction,
                                                 "myplugin.myaction",
                                                 Core::Context(C_GLOBAL));
        cmd->setDefaultKeySequence(QKeySequence(tr("Ctrl+Alt+u")));
        connect(myAction, SIGNAL(triggered()), this, SLOT(performMyAction()));
    \endcode

    So the \c connect is done to your own QAction instance. If you create e.g.
    a tool button that should represent the action you add the action
    from Command::action() to it:
    \code
        QToolButton *myButton = new QToolButton(someParentWidget);
        myButton->setDefaultAction(cmd->action());
    \endcode

    Also use the ActionManager to add items to registered
    action containers like the applications menu bar or menus in that menu bar.
    To do this, you register your action via the
    registerAction functions, get the action container for a specific ID (like specified in
    the Core::Constants namespace) with a call of
    actionContainer(const Id&) and add your command to this container.

    Following the example adding "My Action" to the "Tools" menu would be done by
    \code
        Core::ActionManager::actionContainer(Core::M_TOOLS)->addAction(cmd);
    \endcode

    \section1 Important Guidelines:
    \list
    \li Always register your actions and shortcuts!
    \li Register your actions and shortcuts during your plugin's \l{ExtensionSystem::IPlugin::initialize()}
       or \l{ExtensionSystem::IPlugin::extensionsInitialized()} functions, otherwise the shortcuts won't appear
       in the keyboard settings dialog from the beginning.
    \li When registering an action with \c{cmd=registerAction(action, id, contexts)} be sure to connect
       your own action \c{connect(action, SIGNAL...)} but make \c{cmd->action()} visible to the user, i.e.
       \c{widget->addAction(cmd->action())}.
    \li Use this class to add actions to the applications menus
    \endlist

    \sa Core::ICore
    \sa Core::Command
    \sa Core::ActionContainer
    \sa Core::IContext
*/

/*!
    \fn void ActionManager::commandListChanged()

    Emitted when the command list has changed.
*/

/*!
    \fn void ActionManager::commandAdded(const QString &id)

    Emitted when a command (with the \a id) is added.
*/

static ActionManager *m_instance = 0;
static ActionManagerPrivate *d;

/*!
    \internal
*/
ActionManager::ActionManager(QObject *parent)
    : QObject(parent)
{
    m_instance = this;
    d = new ActionManagerPrivate;
}

/*!
    \internal
*/
ActionManager::~ActionManager()
{
    delete d;
}

/*!
    Returns the pointer to the instance, which is only used for connecting to signals.
*/
ActionManager *ActionManager::instance()
{
    return m_instance;
}

/*!
    Creates a new menu with the given \a id.

    Returns a new ActionContainer that you can use to get the QMenu instance
    or to add menu items to the menu. The ActionManager owns
    the returned ActionContainer.
    Add your menu to some other menu or a menu bar via the
    ActionManager::actionContainer and ActionContainer::addMenu functions.
*/
ActionContainer *ActionManager::createMenu(Id id)
{
    const ActionManagerPrivate::IdContainerMap::const_iterator it = d->m_idContainerMap.constFind(id);
    if (it !=  d->m_idContainerMap.constEnd())
        return it.value();

    QMenu *m = new QMenu(ICore::mainWindow());
    m->setObjectName(QLatin1String(id.name()));

    MenuActionContainer *mc = new MenuActionContainer(id);
    mc->setMenu(m);

    d->m_idContainerMap.insert(id, mc);
    connect(mc, SIGNAL(destroyed()), d, SLOT(containerDestroyed()));

    return mc;
}

/*!
    Creates a new menu bar with the given \a id.

    Returns a new ActionContainer that you can use to get the QMenuBar instance
    or to add menus to the menu bar. The ActionManager owns
    the returned ActionContainer.
*/
ActionContainer *ActionManager::createMenuBar(Id id)
{
    const ActionManagerPrivate::IdContainerMap::const_iterator it = d->m_idContainerMap.constFind(id);
    if (it !=  d->m_idContainerMap.constEnd())
        return it.value();

    QMenuBar *mb = new QMenuBar; // No parent (System menu bar on Mac OS X)
    mb->setObjectName(id.toString());

    MenuBarActionContainer *mbc = new MenuBarActionContainer(id);
    mbc->setMenuBar(mb);

    d->m_idContainerMap.insert(id, mbc);
    connect(mbc, SIGNAL(destroyed()), d, SLOT(containerDestroyed()));

    return mbc;
}

/*!
    Makes an \a action known to the system under the specified \a id.

    Returns a command object that represents the action in the application and is
    owned by the ActionManager. You can register several actions with the
    same \a id as long as the \a context is different. In this case
    a trigger of the actual action is forwarded to the registered QAction
    for the currently active context.
    A scriptable action can be called from a script without the need for the user
    to interact with it.
*/
Command *ActionManager::registerAction(QAction *action, Id id, const Context &context, bool scriptable)
{
    Action *a = d->overridableAction(id);
    if (a) {
        a->addOverrideAction(action, context, scriptable);
        emit m_instance->commandListChanged();
        emit m_instance->commandAdded(id.toString());
    }
    return a;
}

/*!
    Returns the Command object that is known to the system
    under the given \a id.

    \sa ActionManager::registerAction()
*/
Command *ActionManager::command(Id id)
{
    const ActionManagerPrivate::IdCmdMap::const_iterator it = d->m_idCmdMap.constFind(id);
    if (it == d->m_idCmdMap.constEnd()) {
        if (warnAboutFindFailures)
            qWarning() << "ActionManagerPrivate::command(): failed to find :"
                       << id.name();
        return 0;
    }
    return it.value();
}

/*!
    Returns the IActionContainter object that is know to the system
    under the given \a id.

    \sa ActionManager::createMenu()
    \sa ActionManager::createMenuBar()
*/
ActionContainer *ActionManager::actionContainer(Id id)
{
    const ActionManagerPrivate::IdContainerMap::const_iterator it = d->m_idContainerMap.constFind(id);
    if (it == d->m_idContainerMap.constEnd()) {
        if (warnAboutFindFailures)
            qWarning() << "ActionManagerPrivate::actionContainer(): failed to find :"
                       << id.name();
        return 0;
    }
    return it.value();
}

/*!
    Returns all commands that have been registered.
*/
QList<Command *> ActionManager::commands()
{
    // transform list of Action into list of Command
    QList<Command *> result;
    foreach (Command *cmd, d->m_idCmdMap)
        result << cmd;
    return result;
}

/*!
    Removes the knowledge about an \a action under the specified \a id.

    Usually you do not need to unregister actions. The only valid use case for unregistering
    actions, is for actions that represent user definable actions, like for the custom Locator
    filters. If the user removes such an action, it also has to be unregistered from the action manager,
    to make it disappear from shortcut settings etc.
*/
void ActionManager::unregisterAction(QAction *action, Id id)
{
    Action *a = d->m_idCmdMap.value(id, 0);
    if (!a) {
        qWarning() << "unregisterAction: id" << id.name()
                   << "is registered with a different command type.";
        return;
    }
    a->removeOverrideAction(action);
    if (a->isEmpty()) {
        // clean up
        // ActionContainers listen to the commands' destroyed signals
        ICore::mainWindow()->removeAction(a->action());
        delete a->action();
        d->m_idCmdMap.remove(id);
        delete a;
    }
    emit m_instance->commandListChanged();
}

/*!
    Handles the display of the used shortcuts in the presentation mode. The presentation mode is
    enabled when starting \QC with the command line argument \c{-presentationMode}. In the
    presentation mode, \QC displays any pressed shortcut in a grey box.
*/
void ActionManager::setPresentationModeEnabled(bool enabled)
{
    if (enabled == isPresentationModeEnabled())
        return;

    // Signal/slots to commands:
    foreach (Command *c, commands()) {
        if (c->action()) {
            if (enabled)
                connect(c->action(), SIGNAL(triggered()), d, SLOT(actionTriggered()));
            else
                disconnect(c->action(), SIGNAL(triggered()), d, SLOT(actionTriggered()));
        }
    }

    // The label for the shortcuts:
    if (!d->m_presentationLabel) {
        d->m_presentationLabel = new QLabel(0, Qt::ToolTip | Qt::WindowStaysOnTopHint);
        QFont font = d->m_presentationLabel->font();
        font.setPixelSize(45);
        d->m_presentationLabel->setFont(font);
        d->m_presentationLabel->setAlignment(Qt::AlignCenter);
        d->m_presentationLabel->setMargin(5);

        connect(&d->m_presentationLabelTimer, SIGNAL(timeout()), d->m_presentationLabel, SLOT(hide()));
    } else {
        d->m_presentationLabelTimer.stop();
        delete d->m_presentationLabel;
        d->m_presentationLabel = 0;
    }
}

bool ActionManager::isPresentationModeEnabled()
{
    return d->m_presentationLabel;
}

void ActionManager::initialize()
{
    d->initialize();
}

void ActionManager::saveSettings(QSettings *settings)
{
    d->saveSettings(settings);
}

void ActionManager::setContext(const Context &context)
{
    d->setContext(context);
}

/*!
    \class ActionManagerPrivate
    \inheaderfile actionmanager_p.h
    \internal
*/

ActionManagerPrivate::ActionManagerPrivate()
  : m_presentationLabel(0)
{
    m_presentationLabelTimer.setInterval(1000);
}

ActionManagerPrivate::~ActionManagerPrivate()
{
    // first delete containers to avoid them reacting to command deletion
    foreach (ActionContainerPrivate *container, m_idContainerMap)
        disconnect(container, SIGNAL(destroyed()), this, SLOT(containerDestroyed()));
    qDeleteAll(m_idContainerMap);
    qDeleteAll(m_idCmdMap);
}

QDebug operator<<(QDebug d, const Context &context)
{
    d << "CONTEXT: ";
    foreach (Id id, context)
        d << "   " << id.uniqueIdentifier() << " " << id.toString();
    return d;
}

void ActionManagerPrivate::setContext(const Context &context)
{
    // here are possibilities for speed optimization if necessary:
    // let commands (de-)register themselves for contexts
    // and only update commands that are either in old or new contexts
    m_context = context;
    const IdCmdMap::const_iterator cmdcend = m_idCmdMap.constEnd();
    for (IdCmdMap::const_iterator it = m_idCmdMap.constBegin(); it != cmdcend; ++it)
        it.value()->setCurrentContext(m_context);
}

bool ActionManagerPrivate::hasContext(const Context &context) const
{
    for (int i = 0; i < m_context.size(); ++i) {
        if (context.contains(m_context.at(i)))
            return true;
    }
    return false;
}

void ActionManagerPrivate::containerDestroyed()
{
    ActionContainerPrivate *container = static_cast<ActionContainerPrivate *>(sender());
    m_idContainerMap.remove(m_idContainerMap.key(container));
}

void ActionManagerPrivate::actionTriggered()
{
    QAction *action = qobject_cast<QAction *>(QObject::sender());
    if (action)
        showShortcutPopup(action->shortcut().toString());
}

void ActionManagerPrivate::showShortcutPopup(const QString &shortcut)
{
    if (shortcut.isEmpty() || !ActionManager::isPresentationModeEnabled())
        return;

    m_presentationLabel->setText(shortcut);
    m_presentationLabel->adjustSize();

    QPoint p = ICore::mainWindow()->mapToGlobal(ICore::mainWindow()->rect().center() - m_presentationLabel->rect().center());
    m_presentationLabel->move(p);

    m_presentationLabel->show();
    m_presentationLabel->raise();
    m_presentationLabelTimer.start();
}

Action *ActionManagerPrivate::overridableAction(Id id)
{
    Action *a = m_idCmdMap.value(id, 0);
    if (!a) {
        a = new Action(id);
        m_idCmdMap.insert(id, a);
        readUserSettings(id, a);
        ICore::mainWindow()->addAction(a->action());
        a->action()->setObjectName(id.toString());
        a->action()->setShortcutContext(Qt::ApplicationShortcut);
        a->setCurrentContext(m_context);

        if (ActionManager::isPresentationModeEnabled())
            connect(a->action(), SIGNAL(triggered()), this, SLOT(actionTriggered()));
    }

    return a;
}

void ActionManagerPrivate::readUserSettings(Id id, Action *cmd)
{
    QSettings *settings = Core::ICore::settings();
    settings->beginGroup(QLatin1String(kKeyboardSettingsKey));
    if (settings->contains(id.toString()))
        cmd->setKeySequence(QKeySequence(settings->value(id.toString()).toString()));
    settings->endGroup();
}

static const char oldSettingsGroup[] = "KeyBindings";
static const char oldIdKey[] = "ID";
static const char oldSequenceKey[] = "Keysequence";

void ActionManagerPrivate::initialize()
{
    // TODO remove me after some period after 3.1
    // TODO also remove the old settings after some period after 3.1
    // settings->remove(QLatin1String(oldSettingsGroup));
    // settings->contains(QLatin1String(kKeyboardSettingsKey) + QLatin1Char('/')
    //                    + QLatin1String(kKeyboardSettingsTransferredKey))
    // check if settings in old style (pre 3.1) exist
    QSettings *settings = Core::ICore::settings();
    if (settings->contains(QLatin1String(kKeyboardSettingsKey) + QLatin1Char('/')
                           + QLatin1String(kKeyboardSettingsTransferredKey))) {
        return;
    }
    // move old settings style to new settings style
    QMap<Id, QKeySequence> shortcutMap;
    const int shortcuts = settings->beginReadArray(QLatin1String(oldSettingsGroup));
    for (int i = 0; i < shortcuts; ++i) {
        settings->setArrayIndex(i);
        const QKeySequence key(settings->value(QLatin1String(oldSequenceKey)).toString());
        const Id id = Id::fromSetting(settings->value(QLatin1String(oldIdKey)));
        shortcutMap.insert(id, key);
    }
    settings->endArray();
    // write settings in new style
    settings->beginGroup(QLatin1String(kKeyboardSettingsKey));
    settings->setValue(QLatin1String(kKeyboardSettingsTransferredKey), true);
    QMapIterator<Id, QKeySequence> it(shortcutMap);
    while (it.hasNext()) {
        it.next();
        settings->setValue(it.key().toString(), it.value().toString());
    }
    settings->endGroup();
}

void ActionManagerPrivate::saveSettings(QSettings *settings)
{
    settings->beginGroup(QLatin1String(kKeyboardSettingsKey));
    const IdCmdMap::const_iterator cmdcend = m_idCmdMap.constEnd();
    for (IdCmdMap::const_iterator j = m_idCmdMap.constBegin(); j != cmdcend; ++j) {
        const Id id = j.key();
        Action *cmd = j.value();
        QKeySequence key = cmd->keySequence();
        if (key != cmd->defaultKeySequence())
            settings->setValue(id.toString(), key.toString());
        else
            settings->remove(id.toString());
    }
    settings->endGroup();
}
