/**************************************************************************
** Copyright (C) 2010-2019 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2019 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include "queryconstants.hpp"
#include "queryconnection.hpp"
#include "document.hpp"
#include "queryform.hpp"
#include "querywidget.hpp"
#include <qtwrapper/trackingenabled.hpp>
#include <qtwrapper/waitcursor.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/datafile.hpp>
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
#include <QComboBox>
#include <QFileDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QStackedWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>
#include <QTextEdit>
#include <QLabel>
#include <QLineEdit>
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

MainWindow::MainWindow(QWidget *parent) : Utils::FancyMainWindow(parent)
                                        , stack_( new QStackedWidget )
{
}

QWidget *
MainWindow::createContents( Core::IMode * )
{
    setTabPosition( Qt::AllDockWidgetAreas, QTabWidget::North );
    setDocumentMode( true );
    setDockNestingEnabled( true );

    Utils::StyledBar * toolBar1 = createTopStyledBar();

    //---------- central widget ------------
    QWidget * centralWidget = new QWidget;
    setCentralWidget( centralWidget );

    QVBoxLayout * viewLayout = new QVBoxLayout( centralWidget );
    viewLayout->setMargin(0);
    viewLayout->setSpacing(0);
    viewLayout->addWidget( toolBar1 );

    if ( auto widget = new QueryWidget ) {
        viewLayout->addWidget( widget );

        if ( auto btn = findChild< QPushButton * >("btnHistory") )
            connect( btn, &QPushButton::clicked, widget, &QueryWidget::showHistory );
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
        toolBarLayout->setSpacing( 2 );

        if ( auto am = Core::ActionManager::instance() ) {
            Core::Context context( ( Core::Id( "query.MainView" ) ) );

            if ( auto btnOpen = new QToolButton ) {
                btnOpen->setDefaultAction( Core::ActionManager::instance()->command( Constants::FILE_OPEN )->action() );
                btnOpen->setToolTip( tr("Open result file...") );
                toolBarLayout->addWidget( btnOpen );

                auto edit = new QLineEdit;
                edit->setReadOnly( true );
                edit->setObjectName( Constants::editQueryFilename );
                toolBarLayout->addWidget( edit );

                connect( document::instance(), &document::onConnectionChanged
                         , [edit](){ edit->setText( QString::fromStdWString( document::instance()->connection()->filepath() ) ); } );

            }

            if ( auto button = new QPushButton( "History" ) ) {
                button->setObjectName( "btnHistory" );
                toolBarLayout->addWidget( button );
            }
        }


        toolBarLayout->addWidget( new Utils::StyledSeparator );
        toolBarLayout->addItem( new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum) );
    }
    return toolBar;
}

void
MainWindow::onInitialUpdate()
{
    document::instance()->onInitialUpdate();

#if defined Q_OS_LINUX
    auto font = this->font();
    font.setPointSize( 9 );
    setFont( font );
#endif
}

void
MainWindow::onFinalClose()
{
    commit();
    document::instance()->onFinalClose();
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
}

void
MainWindow::handleOpen()
{
    try {
        QString name = QFileDialog::getOpenFileName( this
                                                     , tr( "Open SQLite file" )
                                                     , document::instance()->lastDataDir()
                                                     , tr( "File(*.adfs)|(*)" ) );
        if ( !name.isEmpty() ) {

            qtwrapper::waitCursor wait;

            if ( auto connection = std::make_shared< QueryConnection >() ) {

                if ( connection->connect( name.toStdWString() ) ) {

                    document::instance()->setConnection( connection.get() );
                }
            }

            Core::ModeManager::activateMode( Core::Id( Constants::C_QUERY_MODE ) );
        }
    } catch ( ... ) {
        QMessageBox::warning( this, "Query MainWindow", boost::current_exception_diagnostic_information().c_str() );
    }

}

QDockWidget *
MainWindow::createDockWidget( QWidget * widget, const QString& title, const QString& pageName )
{
    if ( widget->windowTitle().isEmpty() ) // avoid QTC_CHECK warning on console
        widget->setWindowTitle( title );

    if ( widget->objectName().isEmpty() )
        widget->setObjectName( pageName );

    QDockWidget * dockWidget = addDockForWidget( widget );
    dockWidget->setObjectName( pageName.isEmpty() ? widget->objectName() : pageName );

    if ( title.isEmpty() )
        dockWidget->setWindowTitle( widget->objectName() );
    else
        dockWidget->setWindowTitle( title );

    addDockWidget( Qt::TopDockWidgetArea, dockWidget );

    return dockWidget;
}

void
MainWindow::createDockWidgets()
{
}
