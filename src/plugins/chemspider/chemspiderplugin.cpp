#include "chemspiderplugin.hpp"
#include "chemspiderconstants.hpp"

#include <coreplugin/icore.h>
#include <coreplugin/icontext.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>

#include <QtGui/QAction>
#include <QtGui/QMessageBox>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>

#include <QtCore/QtPlugin>

using namespace ChemSpider::Internal;

ChemSpiderPlugin::ChemSpiderPlugin()
{
    // Create your members
}

ChemSpiderPlugin::~ChemSpiderPlugin()
{
    // Unregister objects from the plugin manager's object pool
    // Delete members
}

bool ChemSpiderPlugin::initialize(const QStringList &arguments, QString *errorString)
{
    Q_UNUSED(arguments)
    Q_UNUSED(errorString)
    Core::ActionManager *am = Core::ICore::instance()->actionManager();
    
    QAction *action = new QAction(tr("ChemSpider action"), this);
    QList<int> globalcontext;
	globalcontext << Core::Constants::C_GLOBAL_ID;
    Core::Command *cmd = am->registerAction(action, Constants::ACTION_ID, globalcontext ); // Core::Context(Core::Constants::C_GLOBAL));
    cmd->setDefaultKeySequence(QKeySequence(tr("Ctrl+Alt+Meta+A")));
    connect(action, SIGNAL(triggered()), this, SLOT(triggerAction()));
    
    Core::ActionContainer *menu = am->createMenu(Constants::MENU_ID);
    menu->menu()->setTitle(tr("ChemSpider"));
    menu->addAction(cmd);
    am->actionContainer(Core::Constants::M_TOOLS)->addMenu(menu);
    
    return true;
}

void ChemSpiderPlugin::extensionsInitialized()
{
    // Retrieve objects from the plugin manager's object pool
    // "In the extensionsInitialized method, a plugin can be sure that all
    //  plugins that depend on it are completely initialized."
}

void
ChemSpiderPlugin::triggerAction()
{
    QMessageBox::information(Core::ICore::instance()->mainWindow(),
                             tr("Action triggered"),
                             tr("This is an action from ChemSpider."));
}

Q_EXPORT_PLUGIN2(ChemSpider, ChemSpiderPlugin)

