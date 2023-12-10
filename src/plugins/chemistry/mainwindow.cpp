/**************************************************************************
** Copyright (C) 2010-2024 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2024 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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
#include "chemconnection.hpp"
#include "chemistryconstants.hpp"
#include "chemquery.hpp"
#include "document.hpp"
#include "moltablewnd.hpp"
#include "queryform.hpp"
#include "rxneditform.hpp"
#include "sqleditform.hpp"
#include "pubchemwnd.hpp"

#include <adportable/profile.hpp>
#include <adportable/json_helper.hpp>
#include <adchem/sdfile.hpp>
#include <adwidgets/molview.hpp>
#include <adwidgets/create_widget.hpp>
#include <adwidgets/pugrestform.hpp>
#include <qtwrapper/trackingenabled.hpp>
#include <qtwrapper/waitcursor.hpp>

#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/imode.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/findplaceholder.h>
#include <coreplugin/rightpane.h>
#include <coreplugin/minisplitter.h>
#include <coreplugin/outputpane.h>
#include <coreplugin/navigationwidget.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/icore.h>
#include <utils/styledbar.h>

#include <QByteArray>
#include <QDebug>
#include <QDockWidget>
#include <QFileDialog>
#include <QLineEdit>
#include <QMenu>
#include <QResizeEvent>
#include <qstackedwidget.h>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QLabel>
#include <QtGui/QIcon>
#include <QStandardItemModel>
#include <QProgressBar>

#include <boost/filesystem/path.hpp>
//#include <boost/bind.hpp>
#include <algorithm>

namespace {
    struct ToolButton {
        QToolButton * operator()( QAction * action, const QString& objectName = {})  {
            if ( QToolButton * button = new QToolButton() ) {
                button->setDefaultAction( action );
                if ( !objectName.isEmpty() ) {
                    button->setObjectName( objectName );
                    button->setCheckable( true );
                }
                return button;
            }
            return nullptr;
        }
    };
}

namespace chemistry {

    class MainWindow::impl {
    public:
        static MainWindow * instance_;

        impl( MainWindow * pThis ) : toolBar_( 0 )
                                   , toolBarLayout_( 0 )
                                   , actionSearch_( 0 )
                                   , progressBar_( 0 )
                                   , stackedWidget_( 0 ) {
            instance_ = pThis;
        }

		QWidget * toolBar_;
		QHBoxLayout * toolBarLayout_;
		QAction * actionSearch_;
        QProgressBar * progressBar_;
        QStackedWidget * stackedWidget_;
    };

    MainWindow * MainWindow::impl::instance_ = 0;

}

using namespace chemistry;

MainWindow::~MainWindow()
{
    delete impl_;
}

MainWindow::MainWindow( QWidget * parent ) : Utils::FancyMainWindow( parent )
                                           , impl_( new impl( this ) )
{
}


void
MainWindow::OnInitialUpdate()
{
    connect( document::instance(), &document::onConnectionChanged, [this]{ handleConnectionChanged(); } );

	setSimpleDockWidgetArrangement();
    document::instance()->initialSetup();

#if ! defined Q_OS_MAC
    if ( auto wnd = findChild< MolTableWnd * >() ) {
        wnd->setStyleSheet( "QHeaderView::section {"
                            "  background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1"
                            "    ,stop:0 #616161, stop: 0.5 #505050"
                            "    ,stop: 0.6 #434343, stop:1 #656565 );"
                            "  color: white;"
                            "  padding-left: 4px;"
                            "  border: 1px solid #6c6c6c;"
                            "  font-size: 9pt;"
                            "}"
                            "QHeaderView::section:checked {"
                            "  background-color: gray;"
                            "  font-size: 9pt;"
                            "}"
                            "QTableView {"
                            "  font-size: 9pt;"
                            "}"
            );
    }

    for ( auto dock: dockWidgets() )
        dock->widget()->setStyleSheet( "* { font-size: 9pt; }" );

    for ( auto tabbar: findChildren< QTabBar * >() )
        tabbar->setStyleSheet( "QTabBar { font-size: 9pt }" );
#endif
}

void
MainWindow::OnClose()
{
    document::instance()->finalClose();
}

void
MainWindow::activateLayout()
{
}

void
MainWindow::createActions()
{
	impl_->actionSearch_ = new QAction( QIcon( ":/chemistry/images/search.png" ), tr("Search"), this );
    connect( impl_->actionSearch_, SIGNAL( triggered() ), this, SLOT( actionSearch() ) );
}

QWidget *
MainWindow::createContents()
{
    setTabPosition( Qt::AllDockWidgetAreas, QTabWidget::East );
    setDocumentMode( true );
    setDockNestingEnabled( true );

    Utils::StyledBar * toolBar1 = createTopStyledBar();
    Utils::StyledBar * toolBar2 = createMidStyledBar();

    //---------- centraol widget ------------
    QWidget * centralWidget = new QWidget;
    setCentralWidget( centralWidget );

    impl_->stackedWidget_ = new QStackedWidget;
    impl_->stackedWidget_->addWidget( new MolTableWnd );
    if ( auto wnd = adwidgets::add_widget( impl_->stackedWidget_, adwidgets::create_widget< PubChemWnd >( "PubChem" ) ) ) {
        connect( document::instance(), &document::pugReply, wnd, &PubChemWnd::handleReply );
    }

    if ( Core::MiniSplitter * splitter = new Core::MiniSplitter ) {
        splitter->addWidget( impl_->stackedWidget_ );
        splitter->setStretchFactor( 0, 1 );
        splitter->setStretchFactor( 1, 0 );

        if ( QVBoxLayout * centralLayout = new QVBoxLayout( centralWidget ) ) {
            centralLayout->setContentsMargins( {} );
            centralLayout->setSpacing( 0 );

            centralLayout->addWidget( toolBar1 );
            centralLayout->addWidget( splitter ); // <-- mol table
            centralLayout->addWidget( toolBar2 );
        }
    }

	createDockWidgets();

    if ( auto view = findChild< adwidgets::MolView * >() ) {
        if ( auto wnd = findChild< MolTableWnd * >() ) {
            connect( wnd, &MolTableWnd::activated, [=]( const QModelIndex& index ){
                auto data = wnd->data( index.row(), "svg" );
                view->setData( data );
            });
        }
    }
	return this; // mainWindowSplitter;
}

void
MainWindow::setSimpleDockWidgetArrangement()
{
	qtwrapper::TrackingEnabled<MainWindow> x( *this );

	QList< QDockWidget *> dockWidgets = this->dockWidgets();
    foreach ( QDockWidget * dockWidget, dockWidgets ) {
		dockWidget->setFloating( false );
		removeDockWidget( dockWidget );
	}

    size_t npos = 0;
    foreach ( QDockWidget * dockWidget, dockWidgets ) {
		addDockWidget( Qt::BottomDockWidgetArea, dockWidget );
		dockWidget->show();
        if ( npos++ > 1 )
			tabifyDockWidget( dockWidgets[1], dockWidget );
	}

	update();
}


QDockWidget *
MainWindow::createDockWidget( QWidget * widget, const QString& title, const QString& objname )
{
    if ( widget->windowTitle().isEmpty() ) {
        widget->setWindowTitle( title );
    }

    if ( widget->objectName().isEmpty() ) {
        widget->setObjectName( objname );
    }

	QDockWidget * dockWidget = addDockForWidget( widget );
	dockWidget->setObjectName( widget->objectName() );
    if ( title.isEmpty() )
		dockWidget->setWindowTitle( widget->objectName() );
	else
		dockWidget->setWindowTitle( title );

	addDockWidget( Qt::BottomDockWidgetArea, dockWidget );

	return dockWidget;
}

void
MainWindow::createDockWidgets()
{
    if ( auto w = new adwidgets::MolView( this ) ) {
        createDockWidget( w, "MOL", "MolView" );
    }

    if ( auto w = adwidgets::create_widget< SqlEditForm >( "SqlEditForm", this ) ) {
        if ( auto table = findChild< MolTableWnd * >() ) {
            QObject::connect( w, &SqlEditForm::triggerQuery, table, &MolTableWnd::setQuery );
        }
        createDockWidget( w, "SQL" );
    }

    if ( auto w = adwidgets::create_widget< RxnEditForm >( "RxnEditForm", this ) ) {
        createDockWidget( w, "RXN" );
    }

    if ( auto w = new adwidgets::PUGRestForm( this ) ) {
        createDockWidget( w, "PubChem", "PubChem" );
        connect( w, &adwidgets::PUGRestForm::apply, document::instance(), &document::PubChem );
        connect( w, &adwidgets::PUGRestForm::apply, [&](const QByteArray& ){ impl_->stackedWidget_->setCurrentIndex(1); } );
    }
}

// slot
void
MainWindow::actionSearch()
{
}

MainWindow *
MainWindow::instance()
{
	return impl::instance_;
}

void
MainWindow::handleDropped( const QList< QUrl >& urls )
{
    if ( auto wnd = findChild< MolTableWnd * >() ) {
        for ( auto& url: urls ) {
            std::filesystem::path path( url.toLocalFile().toStdWString() );
            // todo
        }
    }
}

Utils::StyledBar *
MainWindow::createMidStyledBar()
{
    Utils::StyledBar * toolBar = new Utils::StyledBar;
    if ( toolBar ) {
        toolBar->setProperty( "topBorder", true );
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
        toolBarLayout->setContentsMargins( {} );
        toolBarLayout->setSpacing( 0 );

        if ( auto am = Core::ActionManager::instance() ) {
            toolBarLayout->addWidget(ToolButton()(am->command(Constants::SDFILE_OPEN)->action()));
        }

        toolBarLayout->addWidget( new Utils::StyledSeparator );
        toolBarLayout->addItem( new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum) );
        impl_->progressBar_ = new QProgressBar;
		impl_->progressBar_->setVisible( false );
        toolBarLayout->addWidget( impl_->progressBar_ );
        impl_->progressBar_->setStyleSheet( QString("QProgressBar { color: lightgreen }") );
    }
    return toolBar;
}

Utils::StyledBar *
MainWindow::createTopStyledBar()
{
    if ( auto toolBar = new Utils::StyledBar ) {
        toolBar->setProperty( "topBorder", true );
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
        toolBarLayout->setContentsMargins( {} );
        toolBarLayout->setSpacing( 0 );

        if ( auto am = Core::ActionManager::instance() ) {
            Core::Context context( ( Utils::Id( "Chemistry.MainWindow" ) ) );

            if ( auto p = new QAction( tr("Mols"), this ) ) {
                connect( p, &QAction::triggered, [this](){ impl_->stackedWidget_->setCurrentIndex( 0 ); } );
                am->registerAction( p, "Chemistry.selMols", context );
                toolBarLayout->addWidget( ToolButton()( p, QString( "wnd.%1" ).arg( 0 ) ) );
            }

            if ( auto p = new QAction( tr("PubChem"), this ) ) {
                connect( p, &QAction::triggered, [this](){ impl_->stackedWidget_->setCurrentIndex( 1 ); } );
                am->registerAction( p, "Chemistry.selPubChem", context );
                toolBarLayout->addWidget( ToolButton()( p, QString( "wnd.%1" ).arg( 1 ) ) );
            }

            //-- separator --
            toolBarLayout->addWidget( new Utils::StyledSeparator );
        }

        return toolBar;
    }
    return nullptr;
}

void
MainWindow::actSDFileOpen()
{
#if 0
    std::filesystem::path datapath( adportable::profile::user_data_dir<char>() );
    datapath /= "data";

    QString name
        = QFileDialog::getOpenFileName( this
                                        , tr("Open Chemistry database" )
										, datapath.string().c_str()
                                        , tr("Structure Data files(*.sdf);;MDL MOL files(*.mol);;QtPlatz Chem DB(*.adfs)") );
    if ( ! name.isEmpty() ) {

        if ( auto wnd = findChild< MolTableWnd * >() ) {
            qtwrapper::waitCursor wait;
            QFileInfo finfo( name );

            if ( finfo.suffix() == "sdf" || finfo.suffix() == "mol" ) {

                //topLineEdit_->setText( name );
                adchem::SDFile file( name.toStdString() );
                //wnd->setMol( file, *progressBar_ );

            } else if ( finfo.suffix() == "adfs" ) {

                if ( auto connection = std::make_shared< ChemConnection >() ) {

                    if ( connection->connect( name.toStdWString() ) )
                        document::instance()->setConnection( connection.get() );

                }
            }
        }
	}
#endif
}

void
MainWindow::handleConnectionChanged()
{
    if ( auto table = findChild< MolTableWnd * >() ) {
        table->setQuery(
            "SELECT t1.id,svg,synonym,formula,mass,csid,smiles,InChI,InChiKey,SystematicName FROM mols t1 LEFT OUTER JOIN synonyms t2 on t1.id = t2.id" );
    }
}
