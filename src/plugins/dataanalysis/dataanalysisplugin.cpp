#include "dataanalysisplugin.h"

#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/basemode.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/icore.h>
#include <coreplugin/modemanager.h>
#include <coreplugin/uniqueidmanager.h>

#include <QtCore/QDebug>
#include <QtCore/QtPlugin>
#include <QtGui/QAction>
#include <QtGui/QMenu>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>

using namespace DataAnalysis::Internal;

DataAnalysisPlugin::~DataAnalysisPlugin()
{
}

DataAnalysisPlugin::DataAnalysisPlugin()
{
}

void
DataAnalysisPlugin::extensionsInitialized()
{
}

bool
DataAnalysisPlugin::initialize(const QStringList &arguments, QString *error_message)
{
    Q_UNUSED(arguments)
    Q_UNUSED(error_message)

    // Get the primary access point to the workbench.
    Core::ICore *core = Core::ICore::instance();

    // Create a unique context id for our own view, that will be used for the
    // menu entry later.
    QList<int> context = QList<int>()
        << core->uniqueIDManager()->uniqueIdentifier( QLatin1String("DataAnalysis.MainView") );

    // Create an action to be triggered by a menu entry
    QAction *helloWorldAction = new QAction(tr("Say \"&Hello World!\""), this);
    connect(helloWorldAction, SIGNAL(triggered()), SLOT(sayHelloWorld()));

    // Register the action with the action manager
    Core::ActionManager *actionManager = core->actionManager();
    Core::Command *command =
            actionManager->registerAction(
                    helloWorldAction, "DataAnalysis.HelloWorldAction", context);

    // Create our own menu to place in the Tools menu
    Core::ActionContainer *helloWorldMenu =
            actionManager->createMenu("DataAnalysis.HelloWorldMenu");
    QMenu *menu = helloWorldMenu->menu();
    menu->setTitle(tr("&Data Analysis"));
    menu->setEnabled(true);

    // Add the Hello World action command to the menu
    helloWorldMenu->addAction(command);

    // Request the Tools menu and add the Hello World menu to it
    Core::ActionContainer *toolsMenu =
            actionManager->actionContainer(Core::Constants::M_TOOLS);
    toolsMenu->addMenu(helloWorldMenu);

    // Add a mode with a push button based on BaseMode. Like the BaseView,
    // it will unregister itself from the plugin manager when it is deleted.
    Core::BaseMode *baseMode = new Core::BaseMode;
    baseMode->setUniqueModeName("DataAnalysis.HelloWorldMode");
    baseMode->setName(tr("Data Analysis!"));
    baseMode->setIcon(QIcon());
    baseMode->setPriority(0);
    baseMode->setWidget(new QPushButton(tr("Data Analysis PushButton!")));
    baseMode->setContext(context);
    addAutoReleasedObject(baseMode);

    // Add the Hello World action command to the mode manager (with 0 priority)
    Core::ModeManager *modeManager = core->modeManager();
    modeManager->addAction(command, 0);


    return true;
}

void
DataAnalysisPlugin::sayHelloWorld()
{
    QMessageBox::information(0, tr("Data Analysis!"), tr("Hello World! Beutiful day today, isn't it?") );
}

Q_EXPORT_PLUGIN(DataAnalysisPlugin)
