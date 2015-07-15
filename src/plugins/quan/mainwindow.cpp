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
#include "datasequencewidget.hpp"
#include "panelswidget.hpp"
#include "paneldata.hpp"
#include "processmethodwidget.hpp"
#include "quanconstants.hpp"
#include "quanconnection.hpp"
#include "quandocument.hpp"
#include "quanconfigwidget.hpp"
#include "quanresultwnd.hpp"
#include "quanreportwidget.hpp"
#include "quanquerywidget.hpp"
#include <qtwrapper/trackingenabled.hpp>
#include <qtwrapper/waitcursor.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/datafile.hpp>
#include <adcontrols/quansequence.hpp>
#include <adcontrols/quanmethod.hpp>
#include <adcontrols/processmethod.hpp>
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

    tabWidget->addTab( tr( "Quan" ), "", QStringList() << tr( "Select Data" ) << tr( "Compounds & Protocols" ) << tr( "Reports" ) << tr( "Review Result" ) << tr("Query"));
    viewLayout->addWidget( tabWidget );
    tabWidget->setCurrentIndex( 0 );

    viewLayout->addWidget( stack_ );

    auto doc = QuanDocument::instance();
    connect( doc, &QuanDocument::onSequenceCompleted, this, &MainWindow::handleSequenceCompleted );

    // [Select Data] double tab
    
    if ( auto panelsWidget = new PanelsWidget( stack_ ) ) {
        QuanConfigWidget * mw = 0;
        if ( auto widget = new QuanConfigWidget ) {
            // Configuration
            auto panel = std::make_shared< PanelData >( tr("Configuration")
                                                        , QIcon( QLatin1String( ":/quan/images/BuildSettings.png" ) )
                                                        , widget );
            panelsWidget->addPanel( doc->addPanel( 0, 0, panel ) );
            connect( panelsWidget, &PanelsWidget::onCommit, widget, &QuanConfigWidget::commit );
            mw = widget;
        }
        
        if ( auto widget = new DataSequenceWidget ) {
            // Select Data (table)
            auto panel = std::make_shared< PanelData >( tr("Select Data")
                                                        , QIcon( QLatin1String( ":/quan/images/ProjectDependencies.png" ) )
                                                        , widget );
            panelsWidget->addPanel( doc->addPanel( 0, 0, panel ) );
            connect( panelsWidget, &PanelsWidget::onCommit, widget, &DataSequenceWidget::commit );
            connect( mw, &QuanConfigWidget::onLevelChanged, widget, &DataSequenceWidget::handleLevelChaged );
            connect( mw, &QuanConfigWidget::onReplicatesChanged, widget, &DataSequenceWidget::handleReplicatesChanged );
            connect( mw, &QuanConfigWidget::onSampleInletChanged, widget, &DataSequenceWidget::handleSampleInletChanged );
        }
        
        stack_->addWidget( panelsWidget );
    }

    // [Compounds & Protocols] double tab
    
    if ( auto panelsWidget = new PanelsWidget( stack_ ) ) {    
        if ( auto widget = new CompoundsWidget ) {
            auto data = std::make_shared< PanelData >( tr("Compounds")
                                                       , QIcon( QLatin1String( ":/quan/images/unconfigured.png" ) )
                                                       , widget );
            panelsWidget->addPanel( data.get() );
            connect( panelsWidget, &PanelsWidget::onCommit, widget, &CompoundsWidget::commit );
        }

        if ( auto widget = new ProcessMethodWidget ) {
            auto data = std::make_shared< PanelData >( tr("Peak Detection/Assign")
                                                       , QIcon( QLatin1String( ":/quan/images/unconfigured.png" ) )
                                                       , widget );
            panelsWidget->addPanel( data.get() );
            connect( panelsWidget, &PanelsWidget::onCommit, widget, &ProcessMethodWidget::commit );
        }
        stack_->addWidget( panelsWidget );
    }

    if ( auto panelsWidget = new PanelsWidget( stack_ ) ) {        

        if ( auto widget = new QuanReportWidget ) {
            auto data = std::make_shared< PanelData >( "Reports"
                                                       , QIcon( QLatin1String( ":/quan/images/EditorSettings.png" ) )
                                                       , widget );
            panelsWidget->addPanel( data.get() );
            widget->setMaximumHeight( std::numeric_limits<int>::max() );
        }
        stack_->addWidget( panelsWidget );
    }

    // Browse calibration curve & results
    if ( auto panelsWidget = new PanelsWidget( stack_ ) ) {

        if ( auto widget = new QuanResultWnd ) {
            auto data = std::make_shared< PanelData >( "Review Result"
                                                       , QIcon( QLatin1String( ":/quan/images/EditorSettings.png" ) )
                                                       , widget );
            panelsWidget->addPanel( data.get() );
            widget->setMaximumHeight( std::numeric_limits<int>::max() );
        }
        stack_->addWidget( panelsWidget );
    }

    if ( auto panelsWidget = new PanelsWidget( stack_ ) ) {
        if ( auto widget = new QuanQueryWidget ) {
            auto data = std::make_shared< PanelData >( "Query"
                                                       , QIcon( QLatin1String( ":/quan/images/EditorSettings.png" ) )
                                                       , widget );
            widget->setMinimumHeight( 40 );
            panelsWidget->addPanel( data.get() );
        }
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
    QuanDocument::instance()->onInitialUpdate();

    if ( auto rw = findChild< QuanReportWidget * >() )
        rw->onInitialUpdate( QuanDocument::instance() );

    if ( auto qm = QuanDocument::instance()->pm().find< adcontrols::QuanMethod >() ) {
        boost::filesystem::path path = qm->quanMethodFilename();
        if ( !path.empty() ) {
            auto list = findChildren< QLineEdit * >( Constants::editQuanMethodName );
            for ( auto& edit : list ) {
                edit->setText( QString::fromStdWString( path.wstring() ) );
                edit->setEnabled( false );
            }
        }
    }
}

void
MainWindow::onFinalClose()
{
    commit();
    QuanDocument::instance()->onFinalClose();
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
        menu->menu()->setTitle( tr("Quan") );

        Core::Context context( Core::Id( "Quan.MainView" ) );

        if ( auto p = new QAction( QIcon( ":/quan/images/fileopen.png" ), tr( "Open Quan Result..." ), this ) ) {
            am->registerAction( p, Constants::FILE_OPEN, Core::Context( Core::Constants::C_GLOBAL ) );   // Tools->Quan->Open
            connect( p, &QAction::triggered, this, &MainWindow::handleOpenQuanResult );
            menu->addAction( am->command( Constants::FILE_OPEN ) );

            am->registerAction( p, Core::Constants::OPEN, Core::Context( Constants::C_QUAN_MODE ) );  // File->Open
       }

        //------------ method --------------
        if ( auto p = new QAction( QIcon( ":/quan/images/fileopen.png" ), tr( "Open Quan Method..." ), this ) ) {
            am->registerAction( p, Constants::QUAN_METHOD_OPEN, Core::Context( Constants::C_QUAN_MODE ) );
            connect( p, &QAction::triggered, this, &MainWindow::handleOpenQuanMethod );
            menu->addAction( am->command( Constants::QUAN_METHOD_OPEN ) );
        }

        if ( auto p = new QAction( QIcon( ":/quan/images/filesave.png" ), tr( "Save Quan Method..." ), this ) ) {
            am->registerAction( p, Constants::QUAN_METHOD_SAVE, Core::Context( Constants::C_QUAN_MODE ) );
            connect( p, &QAction::triggered, this, &MainWindow::handleSaveQuanMethod );
            menu->addAction( am->command( Constants::QUAN_METHOD_SAVE ) );
        }
        //------------ sequence --------------
        if ( auto p = new QAction( QIcon( ":/quan/images/fileopen.png" ), tr( "Open Quan Sequence..." ), this ) ) {
            am->registerAction( p, Constants::QUAN_SEQUENCE_OPEN, Core::Context( Constants::C_QUAN_MODE ) );
            connect( p, &QAction::triggered, this, &MainWindow::handleOpenQuanSequence );
            menu->addAction( am->command( Constants::QUAN_SEQUENCE_OPEN ) );
        }

        if ( auto p = new QAction( QIcon( ":/quan/images/filesave.png" ), tr( "Save Quan Sequence..." ), this ) ) {
            am->registerAction( p, Constants::QUAN_SEQUENCE_SAVE, Core::Context( Constants::C_QUAN_MODE ) );
            connect( p, &QAction::triggered, this, &MainWindow::handleSaveQuanSequence );
            menu->addAction( am->command( Constants::QUAN_SEQUENCE_SAVE ) );
        }

        if ( auto p = new QAction( QIcon( ":/quan/images/run.png" ), tr("Run"), this ) ) {
            am->registerAction( p, Constants::QUAN_SEQUENCE_RUN, Core::Context( Constants::C_QUAN_MODE ) );
            connect( p, &QAction::triggered, this, &MainWindow::run );
            menu->addAction( am->command( Constants::QUAN_SEQUENCE_RUN ) );
        }

        if ( auto p = new QAction( QIcon(":/quan/images/stop.png"), tr("Stop"), this ) ) {
            am->registerAction( p, Constants::QUAN_SEQUENCE_STOP, Core::Context( Constants::C_QUAN_MODE ) );
            connect( p, &QAction::triggered, this, &MainWindow::stop );
            menu->addAction( am->command( Constants::QUAN_SEQUENCE_STOP ) );
            p->setEnabled( false );
        }

        am->actionContainer( Core::Constants::M_TOOLS )->addMenu( menu );
    }
}

void
MainWindow::handleIndexChanged( int index, int subIndex )
{
    (void)index;
    if ( stack_ ) {
        QWidget * widget = stack_->widget( stack_->currentIndex() );
        if ( auto panels = dynamic_cast< PanelsWidget * >( widget ) )
            panels->commit();
        stack_->setCurrentIndex( subIndex );
    }
}

void
MainWindow::commit()
{
    if ( stack_ ) {
        for ( int idx = 0; idx < stack_->count(); ++idx ) {
            QWidget * widget = stack_->widget( idx );
            if ( auto panels = dynamic_cast< PanelsWidget * >( widget ) )
                panels->commit();
        }
    }
}

void
MainWindow::run()
{
    commit();

    if ( auto qm = QuanDocument::instance()->pm().find< adcontrols::QuanMethod >() ) {
        if ( qm->levels() == 1 && qm->replicates() == 1 ) {
            if ( qm->equation() != adcontrols::QuanMethod::idCalibOnePoint ) {
                QMessageBox::critical( this, "Quan Method Error", "Calibration Eq. does not match with selected levels/replicates." );
                return;
            }
        }
    }

    if ( auto sequence = QuanDocument::instance()->quanSequence() ) {

        if ( sequence->size() == 0 ) {
            QMessageBox::critical( this, "Quan Execution Error", "Empty sample sequence." );
            return;
        }

        boost::filesystem::path path( sequence->outfile() );
        if ( path.empty() ) {
            QMessageBox::critical( this, "Quan Execution Error", "Empty data output filename." );
            return;
        }
        
        if ( boost::filesystem::exists( path ) ) {
            
            QString file( QString::fromStdWString( path.normalize().wstring() ) );
            auto reply = QMessageBox::question( 0, "Quan Sequence Exec"
                                                , QString("File %1% already exists, remove?").arg( file )
                                                , QMessageBox::Yes,QMessageBox::No,QMessageBox::Ignore );
            if ( reply == QMessageBox::No )
                return;

            if ( reply == QMessageBox::Yes ) {
                boost::system::error_code ec;
                boost::filesystem::remove( path, ec );
                if ( ec ) {
                    auto reply = QMessageBox::question( 0, "Quan Sequence Exec"
                                                        , QString( "File %1% cannot be removed. Overwrite?" ).arg( file )
                                                        , QMessageBox::Yes, QMessageBox::No, QMessageBox::Ignore );
                    if ( reply == QMessageBox::No )
                        return;
                }
            }
        }
    }

    if ( auto stop = Core::ActionManager::command(Constants::QUAN_SEQUENCE_STOP)->action() )
        stop->setEnabled( true );

    if ( auto stop = Core::ActionManager::command( Constants::QUAN_SEQUENCE_RUN )->action() )
        stop->setEnabled( false );

    boost::filesystem::path path = QuanDocument::instance()->quanSequence()->outfile();
    Core::DocumentManager::setProjectsDirectory( QString::fromStdWString( path.parent_path().wstring() ) );

    QuanDocument::instance()->run();
}

void
MainWindow::stop()
{
    QuanDocument::instance()->stop();
}

void
MainWindow::handleSequenceCompleted()
{
    if ( auto stop = Core::ActionManager::command( Constants::QUAN_SEQUENCE_STOP )->action() )
        stop->setEnabled( false );
    
    if ( auto stop = Core::ActionManager::command( Constants::QUAN_SEQUENCE_RUN )->action() )
        stop->setEnabled( true );

    if ( auto tab = findChild< DoubleTabWidget * >() )
        tab->setCurrentIndex( -1, 3 );
}

void
MainWindow::handleOpenQuanResult()
{
    try {
        QString name = QFileDialog::getOpenFileName( this
                                                     , tr( "Open Quantitative Analysis Result file" )
                                                     , QuanDocument::instance()->lastDataDir()
                                                     , tr( "File(*.adfs)" ) );
        if ( !name.isEmpty() ) {

            qtwrapper::waitCursor wait;

            if ( auto connection = std::make_shared< QuanConnection >() ) {

                if ( connection->connect( name.toStdWString() ) ) {
                    // kick QuanReportWidget (calibartion & result view) updae
                    QuanDocument::instance()->setConnection( connection.get() );
                }
            }

            if ( auto tab = findChild< DoubleTabWidget * >() )
                tab->setCurrentIndex( -1, 3 );
            Core::ModeManager::activateMode( Core::Id( Constants::C_QUAN_MODE ) );
        }
    }
    catch ( ... ) {
        QMessageBox::warning( this, "Quan MainWindow", boost::current_exception_diagnostic_information().c_str() );
    }
}

void
MainWindow::handleOpenQuanMethod()
{
    auto name = QFileDialog::getOpenFileName( this, tr( "Open Quantitation Method File" )
                                              , QuanDocument::instance()->lastMethodDir()
                                              , tr( "Quan Method Files(*.qmth);;Result Files(*.adfs);;XML Files(*.xml)" ) );
    if ( ! name.isEmpty() ) {
        boost::filesystem::path path( name.toStdWString() );
        adcontrols::ProcessMethod temp;
        if ( QuanDocument::load( path, temp ) ) {
            auto qm = temp.find< adcontrols::QuanMethod >();
            if ( !qm ) {
                temp << adcontrols::QuanMethod();
                qm = temp.find< adcontrols::QuanMethod >();
            }
            qm->quanMethodFilename( path.generic_wstring().c_str() );
            QuanDocument::instance()->replace_method( temp );

            auto list = findChildren< QLineEdit * >( Constants::editQuanMethodName );
            for ( auto& edit : list )
                edit->setText( QString::fromStdWString( path.wstring() ) );
        }
    }
}

void
MainWindow::handleSaveQuanMethod()
{
    auto name = QFileDialog::getSaveFileName( this
                                              , tr( "Save Quantitation Method File" )
                                              , QuanDocument::instance()->lastMethodDir()
                                              , tr( "Quan Method Files(*.qmth);;XML Files(*.xml)" ) );
    if ( !name.isEmpty() ) {

        commit();

        boost::filesystem::path path( name.toStdWString() );
        QuanDocument::save( path, QuanDocument::instance()->pm() );

        if ( auto qm = QuanDocument::instance()->pm().find< adcontrols::QuanMethod >() ) {
            if ( qm->quanMethodFilename() != path ) {
                // update filename on method
                qm->quanMethodFilename( path.generic_wstring().c_str() );

                // update filename on UI
                auto list = findChildren< QLineEdit * >( Constants::editQuanMethodName );
                for ( auto& edit : list )
                    edit->setText( QString::fromStdWString( path.wstring() ) );

            }
        }

    }
}

void
MainWindow::handleOpenQuanSequence()
{
    QString name = QFileDialog::getOpenFileName( this
                                                 , tr( "Open Quantitation Method File" )
                                                 , QuanDocument::instance()->lastSequenceDir()
                                                 , tr( "Quan Sequence Files(*.qseq)" ) );

    if ( ! name.isEmpty() ) {

        boost::filesystem::path path( name.toStdWString() );
        auto seq = std::make_shared< adcontrols::QuanSequence >();

        if ( QuanDocument::instance()->load( path, *seq ) ) {
            seq->filename( path.generic_wstring().c_str() );
            QuanDocument::instance()->quanSequence( seq );
        }

    }
}

void
MainWindow::handleSaveQuanSequence()
{
    QString name = QFileDialog::getSaveFileName( this
                                                 , tr( "Save Quan Sequence File" )
                                                 , QuanDocument::instance()->lastSequenceDir()
                                                 , tr( "Quan Sequence Files(*.qseq)" ) );
    if ( ! name.isEmpty() ) {

        commit(); // commit all tabs

        boost::filesystem::path path( name.toStdWString() );
        QuanDocument::instance()->save( path, *QuanDocument::instance()->quanSequence(), true );
    }
}

void
MainWindow::handleRecentFiles()
{
}
