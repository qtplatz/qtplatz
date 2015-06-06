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
//#include "doubletabwidget.hpp"
//#include "datasequencewidget.hpp"
//#include "panelswidget.hpp"
//#include "paneldata.hpp"
//#include "processmethodwidget.hpp"
#include "queryconstants.hpp"
#include "queryconnection.hpp"
#include "querydocument.hpp"
//#include "queryconfigwidget.hpp"
//#include "queryresultwnd.hpp"
//#include "queryreportwidget.hpp"
#include "queryquerywidget.hpp"
#include <qtwrapper/trackingenabled.hpp>
#include <qtwrapper/waitcursor.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/datafile.hpp>
//#include <adcontrols/querysequence.hpp>
//#include <adcontrols/querymethod.hpp>
//#include <adcontrols/processmethod.hpp>
#include <adportable/profile.hpp>
#include <adlog/logger.hpp>
#include <adportable/debug.hpp>
#include <coreplugin/coreconstants.h>
#include <coreplugin/icore.h>
#include <coreplugin/id.h>
#include <coreplugin/minisplitter.h>
#include <coreplugin/modemanager.h>
#include <coreplugin/rightpane.h>
#include <coreplugin/outputpane.h>
#include <coreplugin/navigationwidget.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/documentmanager.h>
#include <utils/styledbar.h>

#include <QFileDialog>
#include <QLineEdit>
#include <QMessageBox>
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
#include <QMessageBox>

#include <boost/filesystem.hpp>
#include <boost/exception/all.hpp>
#include <numeric>

using namespace query;

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

    if ( auto widget = new QueryQueryWidget ) {
        viewLayout->addWidget( widget );
    }

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
        Core::ActionManager * am = Core::ActionManager::instance(); // ->actionManager();
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
    QueryDocument::instance()->onInitialUpdate();
#if 0
    if ( auto rw = findChild< QueryReportWidget * >() )
        rw->onInitialUpdate( QueryDocument::instance() );

    if ( auto qm = QueryDocument::instance()->pm().find< adcontrols::QueryMethod >() ) {
        boost::filesystem::path path = qm->queryMethodFilename();
        if ( !path.empty() ) {
            auto list = findChildren< QLineEdit * >( Constants::editQueryMethodName );
            for ( auto& edit : list ) {
                edit->setText( QString::fromStdWString( path.wstring() ) );
                edit->setEnabled( false );
            }
        }
    }
#endif
}

void
MainWindow::onFinalClose()
{
    commit();
    QueryDocument::instance()->onFinalClose();
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
    return toolButton( Core::ActionManager::instance()->command(id)->action() );
}

void
MainWindow::createActions()
{
    if ( Core::ActionManager * am = Core::ActionManager::instance() ) {
        
        Core::ActionContainer * menu = am->createMenu( Constants::MENU_ID ); // Menu ID
        menu->menu()->setTitle( tr("Query") );

        if ( auto p = new QAction( QIcon( ":/query/images/fileopen.png" ), tr( "Open SQLite file..." ), this ) ) {

            am->registerAction( p, Constants::FILE_OPEN, Core::Context( Core::Constants::C_GLOBAL ) );   // Tools|Query|Open SQLite file...
            connect( p, &QAction::triggered, this, &MainWindow::handleOpen );
            menu->addAction( am->command( Constants::FILE_OPEN ) );
            am->registerAction( p, Core::Constants::OPEN, Core::Context( Constants::C_QUERY_MODE ) );    // File|Open
       }

        am->actionContainer( Core::Constants::M_TOOLS )->addMenu( menu );
    }
}

void
MainWindow::handleIndexChanged( int index, int subIndex )
{
    (void)index;
    if ( stack_ ) {
#if 0
        QWidget * widget = stack_->widget( stack_->currentIndex() );
        if ( auto panels = dynamic_cast< PanelsWidget * >( widget ) )
            panels->commit();
#endif
        stack_->setCurrentIndex( subIndex );
    }
}

void
MainWindow::commit()
{
#if 0
    if ( stack_ ) {
        for ( int idx = 0; idx < stack_->count(); ++idx ) {
            QWidget * widget = stack_->widget( idx );
            if ( auto panels = dynamic_cast< PanelsWidget * >( widget ) )
                panels->commit();
        }
    }
#endif
}

void
MainWindow::handleOpen()
{
    try {
        QString name = QFileDialog::getOpenFileName( this
                                                     , tr( "Open Quantitative Analysis Result file" )
                                                     , QueryDocument::instance()->lastDataDir()
                                                     , tr( "File(*.adfs)|(*)" ) );
        if ( !name.isEmpty() ) {

            qtwrapper::waitCursor wait;

            if ( auto connection = std::make_shared< QueryConnection >() ) {

                if ( connection->connect( name.toStdWString() ) ) {
                    // kick QuanReportWidget (calibartion & result view) updae
                    QueryDocument::instance()->setConnection( connection.get() );
                }
            }

            Core::ModeManager::activateMode( Core::Id( Constants::C_QUERY_MODE ) );
        }
    } catch ( ... ) {
        QMessageBox::warning( this, "Query MainWindow", boost::current_exception_diagnostic_information().c_str() );
    }
    
}

