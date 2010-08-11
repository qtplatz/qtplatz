//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#include "sequenceplugin.h"
#include "sequencemode.h"
#include "sequenceeditorfactory.h"
#include "sequencemanager.h"

#include <QtCore/qplugin.h>
#include <coreplugin/icore.h>
#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/mimedatabase.h>

#include <coreplugin/minisplitter.h>
#include <coreplugin/outputpane.h>
#include <coreplugin/navigationwidget.h>
#include <coreplugin/rightpane.h>
#include <coreplugin/actionmanager/actionmanager.h>

#include <utils/styledbar.h>
#include <utils/fancymainwindow.h>

#include <QtGui/QHBoxLayout>
#include <QtGui/QBoxLayout>
#include <QtGui/QToolButton>
#include <QtGui/QLabel>
#include <QTableWidget>
#include <QTextEdit>
#include <QToolButton>

#include <QStringList>

using namespace sequence;
using namespace sequence::internal;

SequencePlugin::~SequencePlugin()
{
}

SequencePlugin::SequencePlugin()
{
}

bool
SequencePlugin::initialize(const QStringList& arguments, QString* error_message)
{
    Q_UNUSED( arguments );
    
    Core::ICore * core = Core::ICore::instance();
    
    QList<int> context;
    if ( core ) {
        Core::UniqueIDManager * uidm = core->uniqueIDManager();
        if ( uidm ) {
            context.append( uidm->uniqueIdentifier( QLatin1String("Sequence.MainView") ) );
            context.append( uidm->uniqueIdentifier( Core::Constants::C_NAVIGATION_PANE ) );
        }
    } else
        return false;
    
    Core::MimeDatabase* mdb = core->mimeDatabase();
    if ( mdb ) {
        if ( !mdb->addMimeTypes(":/sequence/sequence-mimetype.xml", error_message) )
            return false;
        addAutoReleasedObject( new SequenceEditorFactory(this) );
    }
    
    SequenceMode * mode = new SequenceMode(this);
    if ( mode )
        mode->setContext( context );
    else
        return false;
    
    manager_.reset( new SequenceManager(0) );
    if ( manager_ )
        manager_->init();
    
    
    //              [mainWindow]
    // splitter> ---------------------
    //              [OutputPane]
    
    Core::MiniSplitter * splitter = new Core::MiniSplitter;
    if ( splitter ) {
        splitter->addWidget( manager_->mainWindow() );
        splitter->addWidget( new Core::OutputPanePlaceHolder( mode ) );
        
        splitter->setStretchFactor( 0, 10 );
        splitter->setStretchFactor( 1, 0 );
        splitter->setOrientation( Qt::Vertical ); // horizontal splitter bar
    }
    
    //////////////////////////////////////////////////////////////
    //
    //         <splitter2>         [mainWindow]
    // [Navigation] | [splitter ------------------- ]
    //                             [OutputPane]
    Core::MiniSplitter * splitter2 = new Core::MiniSplitter;
    if ( splitter2 ) {
        splitter2->addWidget( new Core::NavigationWidgetPlaceHolder( mode ) );
        splitter2->addWidget( splitter );
        splitter2->setStretchFactor( 0, 0 );
        splitter2->setStretchFactor( 1, 1 );
    }
    
    Utils::StyledBar * toolBar = new Utils::StyledBar;
    if ( toolBar ) {
        toolBar->setProperty( "topBorder", true );
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
        toolBarLayout->setMargin(0);
        toolBarLayout->setSpacing(0);
        Core::ActionManager *am = core->actionManager();
        if ( am ) {
            /*
              toolBarLayout->addWidget(toolButton(am->command(Constants::CONNECT)->action()));
              toolBarLayout->addWidget(toolButton(am->command(Constants::INITIALRUN)->action()));
              toolBarLayout->addWidget(toolButton(am->command(Constants::RUN)->action()));
              toolBarLayout->addWidget(toolButton(am->command(Constants::STOP)->action()));
              toolBarLayout->addWidget(toolButton(am->command(Constants::ACQUISITION)->action()));
            */
        }
        toolBarLayout->addWidget( new Utils::StyledSeparator );
        toolBarLayout->addWidget( new QLabel( tr("Sequence:") ) );
    }
    Utils::StyledBar * toolBar2 = new Utils::StyledBar;
    if ( toolBar2 ) {
        toolBar2->setProperty( "topBorder", true );
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar2 );
        toolBarLayout->setMargin(0);
        toolBarLayout->setSpacing(0);
        Core::ActionManager *am = core->actionManager();
        if ( am ) {
            toolBarLayout->addWidget( new QLabel( tr("AA") ) );
            toolBarLayout->addWidget( new Utils::StyledSeparator );
            toolBarLayout->addWidget( new QLabel( tr("BB") ) );
            toolBarLayout->addWidget( new Utils::StyledSeparator );
            toolBarLayout->addWidget( new QLabel( tr("CC") ) );
        }
        toolBarLayout->addWidget( new Utils::StyledSeparator );
        toolBarLayout->addWidget( new QLabel( tr("Threads:") ) );
    }
    
    /******************************************************************************
     */
    
    QWidget* centralWidget = new QWidget;
    manager_->mainWindow()->setCentralWidget( centralWidget );
    
    Core::MiniSplitter * splitter3 = new Core::MiniSplitter;
    if ( splitter3 ) {
        // QTabWidget * pTab = new QTabWidget;
        //splitter3->addWidget( pTab );
        //pTab->addTab( new QTextEdit, QIcon(":/acquire/images/watchpoint.png"), "Sequence" );
        splitter3->addWidget( new QTextEdit );
    }
    
    QBoxLayout * toolBarAddingLayout = new QVBoxLayout( centralWidget );
    toolBarAddingLayout->setMargin(0);
    toolBarAddingLayout->setSpacing(0);
    toolBarAddingLayout->addWidget( toolBar );
    toolBarAddingLayout->addWidget( splitter3 );
    toolBarAddingLayout->addWidget( toolBar2 );
    
    mode->setWidget( splitter2 );
    
    //////////////////////////////////
    manager_->setSimpleDockWidgetArrangement();
    addAutoReleasedObject(mode);
    //////////////////////////////////

    return true;
}

void
SequencePlugin::extensionsInitialized()
{
}

Q_EXPORT_PLUGIN( SequencePlugin )
