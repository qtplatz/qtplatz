/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/

#include "mainwindow.hpp"
#include "compoundswidget.hpp"
#include "doubletabwidget.hpp"
#include "panelswidget.hpp"
#include "paneldata.hpp"
#include "quanconstants.hpp"
#include "quandocument.hpp"
//#include "dataselectionwidget.hpp"
#include "quanconfigwidget.hpp"
#include "datasequencewidget.hpp"
#include <qtwrapper/trackingenabled.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/datafile.hpp>
#include <adportable/profile.hpp>
#include <adlog/logger.hpp>
#include <adportable/debug.hpp>
#include <coreplugin/coreconstants.h>
#include <coreplugin/icore.h>
#include <coreplugin/minisplitter.h>
#include <coreplugin/rightpane.h>
#include <coreplugin/outputpane.h>
#include <coreplugin/navigationwidget.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <utils/styledbar.h>

#include <QFileDialog>
#include <QLineEdit>
#include <QStackedWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qlineedit.h>
#include <QTextEdit>
#include <QTableView>
#include <QDockWidget>
#include <QStandardItemModel>
#include <QMenu>

#include <boost/filesystem.hpp>

using namespace quan;

MainWindow::~MainWindow()
{
}

MainWindow::MainWindow(QWidget *parent) : QWidget( parent )
                                        , stack_( new QStackedWidget )
{
}

QWidget *
MainWindow::createContents( Core::IMode * )
{
    QVBoxLayout * viewLayout = new QVBoxLayout( this );
    viewLayout->setMargin(0);
    viewLayout->setSpacing(0);
    
    auto tabWidget = new DoubleTabWidget( this );

    connect( tabWidget, &DoubleTabWidget::currentIndexChanged, this, &MainWindow::handleIndexChanged );

    tabWidget->addTab( "Quan", "", QStringList() << "Select Data" << "Compounds & Protocols" << "Reports");
    viewLayout->addWidget( tabWidget );
    tabWidget->setCurrentIndex( 0 );

    viewLayout->addWidget( stack_ );

    auto doc = QuanDocument::instance();

    if ( auto panelsWidget = new PanelsWidget( stack_ ) ) {
        auto panel = std::make_shared< PanelData >( "Configuration"
                                                    , QIcon( QLatin1String( ":/quan/images/BuildSettings.png" ) )
                                                    , new QuanConfigWidget );
        panelsWidget->addPanel( doc->addPanel( 0, 0, panel ) );
        
        panel = std::make_shared< PanelData >( "Select Data"
                                               , QIcon( QLatin1String( ":/quan/images/ProjectDependencies.png" ) )
                                               , new DataSequenceWidget );
        
        panelsWidget->addPanel( doc->addPanel( 0, 0, panel ) );        
        
        stack_->addWidget( panelsWidget );
    }
    
    if ( auto panelsWidget = new PanelsWidget( stack_ ) ) {    
        auto data = std::make_shared< PanelData >( "Compounds"
                                                   , QIcon( QLatin1String( ":/quan/images/unconfigured.png" ) )
                                                   , new CompoundsWidget );
        panelsWidget->addPanel( data.get() );
        stack_->addWidget( panelsWidget );
    }

    if ( auto panelsWidget = new PanelsWidget( stack_ ) ) {        
        auto data = std::make_shared< PanelData >( "Reports"
                                                   , QIcon( QLatin1String( ":/quan/images/EditorSettings.png" ) )
                                                   , new QTextEdit );
        panelsWidget->addPanel( data.get() );
        stack_->addWidget( panelsWidget );
    }

    stack_->setCurrentIndex( 0 );
    
    return this;
}

Utils::StyledBar *
MainWindow::createTopStyledBar()
{
    Utils::StyledBar * toolBar = new Utils::StyledBar;
    if ( toolBar ) {
        toolBar->setProperty( "topBorder", true );
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
        toolBarLayout->setMargin( 0 );
        toolBarLayout->setSpacing( 0 );
        Core::ActionManager * am = Core::ICore::instance()->actionManager();
        if ( am ) {
            // [file open] button
            //toolBarLayout->addWidget( toolButton( am->command( Constants::FILE_OPEN )->action() ) );
            //-- separator --
            toolBarLayout->addWidget( new Utils::StyledSeparator );
            //---
            //toolBarLayout->addWidget( topLineEdit_.get() );
        }
        toolBarLayout->addWidget( new Utils::StyledSeparator );
        toolBarLayout->addItem( new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum) );
    }
    return toolBar;
}

void
MainWindow::onInitialUpdate()
{
}

// static
QToolButton * 
MainWindow::toolButton( QAction * action )
{
    QToolButton * button = new QToolButton;
    if ( button )
        button->setDefaultAction( action );
    return button;
}

// static
QToolButton * 
MainWindow::toolButton( const char * id )
{
    Core::ActionManager * mgr = Core::ICore::instance()->actionManager();
    return toolButton( mgr->command(id)->action() );
}

QAction *
MainWindow::createAction( const QString& iconname, const QString& msg, QObject * parent )
{
    QIcon icon;
    icon.addFile( iconname );
    return new QAction( icon, msg, parent );
}

void
MainWindow::createActions()
{
    //actions_[ idActFileOpen ] = createAction( Constants::ICON_FILE_OPEN, tr("Open protain file..."), this );
    //connect( actions_[ idActFileOpen ], SIGNAL( triggered() ), this, SLOT( actFileOpen() ) );

    const QList<int> gc = QList<int>() << Core::Constants::C_GLOBAL_ID;

    if ( Core::ActionManager * am = Core::ICore::instance()->actionManager() ) {
        
        Core::ActionContainer * menu = am->createMenu( Constants::MENU_ID ); // Menu ID
        menu->menu()->setTitle( "Quan" );
        
        //Core::Command * cmd = 0;

        //cmd = am->registerAction( actions_[ idActFileOpen ], Constants::FILE_OPEN, gc );
        //menu->addAction( cmd );

        am->actionContainer( Core::Constants::M_TOOLS )->addMenu( menu );
    }
}

void
MainWindow::handleIndexChanged( int index, int subIndex )
{
    (void)index;
    if ( stack_ )
        stack_->setCurrentIndex( subIndex );
}
