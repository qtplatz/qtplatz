#include "example.hpp"
#include "exampleconstants.hpp"

#include <coreplugin/icore.h>
#include <coreplugin/icontext.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>
#include <iostream>

#include <QAction>
#include <QMessageBox>
#include <QMainWindow>
#include <QMenu>

namespace Example {
namespace Internal {

ExamplePlugin::ExamplePlugin()
{
    std::cout << "---------------- ExamplePlugin ctor -------------------------\n";
}

ExamplePlugin::~ExamplePlugin()
{
    // Unregister objects from the plugin manager's object pool
    // Delete members
    std::cout << "---------------- ExamplePlugin dtor -------------------------\n";
}

bool ExamplePlugin::initialize(const QStringList &arguments, QString *errorString)
{
    // Register objects in the plugin manager's object pool
    // Load settings
    // Add actions to menus
    // Connect to other plugins' signals
    // In the initialize function, a plugin can be sure that the plugins it
    // depends on have initialized their members.
    std::cout << "---------------- ExamplePlugin::initialize -------------------------\n";

    Q_UNUSED(arguments)
    Q_UNUSED(errorString)

    auto action = new QAction(tr("Example Action"), this);
    Core::Command *cmd = Core::ActionManager::registerAction(action, Constants::ACTION_ID,
                                                             Core::Context(Core::Constants::C_GLOBAL));
    cmd->setDefaultKeySequence(QKeySequence(tr("Ctrl+Alt+Meta+A")));
    connect(action, &QAction::triggered, this, &ExamplePlugin::triggerAction);

    Core::ActionContainer *menu = Core::ActionManager::createMenu(Constants::MENU_ID);
    menu->menu()->setTitle(tr("Example"));
    menu->addAction(cmd);
    Core::ActionManager::actionContainer(Core::Constants::M_TOOLS)->addMenu(menu);

    return true;
}

void ExamplePlugin::extensionsInitialized()
{
    std::cout << "---------------- ExamplePlugin::extensionsInitialized -------------------------\n";
    // Retrieve objects from the plugin manager's object pool
    // In the extensionsInitialized function, a plugin can be sure that all
    // plugins that depend on it are completely initialized.
}

ExtensionSystem::IPlugin::ShutdownFlag ExamplePlugin::aboutToShutdown()
{
    std::cout << "---------------- ExamplePlugin::aboutToShutdown -------------------------\n";
    // Save settings
    // Disconnect from signals that are not needed during shutdown
    // Hide UI (if you add UI that is not in the main window directly)
    return SynchronousShutdown;
}

void ExamplePlugin::triggerAction()
{
    QMessageBox::information(Core::ICore::mainWindow(),
                             tr("Action Triggered"),
                             tr("This is an action from Example."));
}

} // namespace Internal
} // namespace Example
