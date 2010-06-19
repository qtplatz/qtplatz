#include "dataanalysisplugin.h"
#include "dataanalysismanager.h"

#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/basemode.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/icore.h>
#include <coreplugin/modemanager.h>
#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/minisplitter.h>
#include <coreplugin/rightpane.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/findplaceholder.h>
#include <coreplugin/outputpane.h>
#include <coreplugin/navigationwidget.h>
#include <utils/fancymainwindow.h>

#include <QtCore/QDebug>
#include <QtCore/QtPlugin>
#include <QtGui/QAction>
#include <QtGui/QMenu>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>
#include "dataeditor.h"
#include "dataeditorfactory.h"
#include "dataanalysiswindow.h"
#include <coreplugin/mimedatabase.h>
#include "openeditorsview.h"

#include <coreplugin/editormanager/editormanager.h>

using namespace DataAnalysis::Internal;

DataAnalysisPlugin::~DataAnalysisPlugin()
{
}

DataAnalysisPlugin::DataAnalysisPlugin() : manager_(0)
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
    ;

  manager_ = new DataAnalysisManager;
  if ( manager_ )
      manager_->init();

  addAutoReleasedObject( new OpenEditorsViewFactory() );

  // Get the primary access point to the workbench.
  Core::ICore *core = Core::ICore::instance();
  if ( !core )
    return false;
  
  do {
    Core::MimeDatabase * mdb = core->mimeDatabase();
    if ( mdb )
      mdb->addMimeTypes( QLatin1String(":/dataanalysis/dataaalysis.mimetypes.xml"), error_message);
  } while (0);

  do {
    DataEditorFactory * factory = new DataEditorFactory(this);
    addAutoReleasedObject( factory );
  } while (0);
  
  // Create a unique context id for our own view, that will be used for the
  // menu entry later.
  QList<int> context;
  context.append( core->uniqueIDManager()->uniqueIdentifier( Core::Constants::C_EDITORMANAGER ) );
  context.append( core->uniqueIDManager()->uniqueIdentifier( QLatin1String("DataAnalysis.MainView") ) );
  context.append( core->uniqueIDManager()->uniqueIdentifier( Core::Constants::C_NAVIGATION_PANE ) );

  DataAnalysisMode * mode = new DataAnalysisMode(this);
  mode->setContext( context );

  QWidget * editAndFindWidget = new QWidget;
  if ( editAndFindWidget ) {
    //------------
    QBoxLayout * editorHolderLayout = new QVBoxLayout;
    editorHolderLayout->setMargin(0);
    editorHolderLayout->addWidget( new Core::EditorManagerPlaceHolder(mode) );
    editorHolderLayout->addWidget( new Core::FindToolBarPlaceHolder( editAndFindWidget ) );
    editAndFindWidget->setLayout( editorHolderLayout );
  }

  //------------
  Core::MiniSplitter * rightPaneSplitter = new Core::MiniSplitter;
  if ( rightPaneSplitter ) {
    rightPaneSplitter->addWidget( editAndFindWidget );
    rightPaneSplitter->addWidget( new Core::RightPanePlaceHolder( mode ) );
    rightPaneSplitter->setStretchFactor( 0, 1 );
    rightPaneSplitter->setStretchFactor( 1, 0 );
  }
  
  //------------
  QWidget * centralWidget = new QWidget;
  manager_->mainWindow()->setCentralWidget( centralWidget );

  Core::MiniSplitter * splitter = new Core::MiniSplitter;
  splitter->addWidget( manager_->mainWindow() );
  splitter->addWidget( new Core::OutputPanePlaceHolder( mode ) );
  splitter->setStretchFactor( 0, 10 );
  splitter->setStretchFactor( 1, 0 );
  splitter->setOrientation( Qt::Vertical );

  Core::MiniSplitter * splitter2 = new Core::MiniSplitter;
  splitter2->addWidget( new Core::NavigationWidgetPlaceHolder( mode ) );
  splitter2->addWidget( splitter );
  splitter2->setStretchFactor( 0, 0 );
  splitter2->setStretchFactor( 1, 1 );

  mode->setWidget( splitter2 );

  Core::Command * command(0);
  do {
    // Create an action to be triggered by a menu entry
    QAction *helloWorldAction = new QAction(tr("Say \"&Hello World!\""), this);
    connect(helloWorldAction, SIGNAL(triggered()), SLOT(sayHelloWorld()));
    
  // Register the action with the action manager

    Core::ActionManager *actionManager = core->actionManager();
    command = actionManager->registerAction(helloWorldAction, "DataAnalysis.HelloWorldAction", context);
    
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
  } while (0); // End register to the action manager
  
  // Add a mode with a push button based on BaseMode. Like the BaseView,
  // it will unregister itself from the plugin manager when it is deleted.

  //    baseMode->setWidget(new QPushButton(tr("Data Analysis PushButton!")));
  // mode->setWidget( new DataAnalysisWindow( 0 ) );
  // mode->setContext(context);
  addAutoReleasedObject(mode);

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


////////////////////////

DataAnalysisMode::~DataAnalysisMode()
{
    Core::EditorManager::instance()->setParent(0);
}

DataAnalysisMode::DataAnalysisMode( QObject * parent ) : Core::BaseMode( parent )
{
    setName(tr("Data Analysis"));
    setUniqueModeName( "DataAnalysisr.Mode" );
    setIcon(QIcon(":/fancyactionbar/images/mode_Debug.png"));
    setPriority( 85 );
}
