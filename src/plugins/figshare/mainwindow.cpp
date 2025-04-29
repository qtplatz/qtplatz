// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "mainwindow.hpp"
#include "restwnd.hpp"
#include "document.hpp"
#include "csvwnd.hpp"
#include <QtWidgets/qlayoutitem.h>
#include <adportable/debug.hpp>
#include <adwidgets/create_widget.hpp>
//#include <adwidgets/pugrestform.hpp>
#include <adwidgets/jstrestform.hpp>
#include <adwidgets/figsharerestform.hpp>
#include <qtwrapper/trackingenabled.hpp>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/findplaceholder.h>
#include <coreplugin/icore.h>
#include <coreplugin/imode.h>
#include <coreplugin/minisplitter.h>
#include <coreplugin/minisplitter.h>
#include <coreplugin/navigationwidget.h>
#include <coreplugin/outputpane.h>
#include <coreplugin/rightpane.h>
#include <utils/styledbar.h>

#include <QToolButton>
#include <QTextEdit>
#include <QTabBar>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QDockWidget>
#include <format>

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

namespace figshare {

    class MainWindow::impl {
    public:
        impl( MainWindow * p ) : toolBar_( 0 )
                               , toolBarLayout_( 0 )
                               , action_( 0 )
                               , stackedWidget_( 0 ) {
            instance_ = p;
        }

		QWidget * toolBar_;
		QHBoxLayout * toolBarLayout_;
		QAction * action_;
        QStackedWidget * stackedWidget_;

        static MainWindow * instance_;
    };

    MainWindow * MainWindow::impl::instance_ = nullptr;

}

using namespace figshare;

MainWindow::MainWindow(QWidget *parent) : Utils::FancyMainWindow(parent)
                                        , impl_( new impl( this ) )
{
}

void
MainWindow::OnClose()
{
}

void
MainWindow::OnInitialUpdate()
{
    ADDEBUG() << __FUNCTION__;

	setSimpleDockWidgetArrangement();

    ADDEBUG() << __FUNCTION__;

#if ! defined Q_OS_MAC
    if ( auto wnd = findChild< RESTWnd * >() ) {
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
        if ( npos++ > 0 )
            tabifyDockWidget( dockWidgets[0], dockWidget );
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
    if ( title.isEmpty() ) {
		dockWidget->setWindowTitle( widget->objectName() );
    } else {
		dockWidget->setWindowTitle( title );
    }

	addDockWidget( Qt::BottomDockWidgetArea, dockWidget );

	return dockWidget;
}

void
MainWindow::createActions()
{
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

    if ( auto wnd = adwidgets::add_widget( impl_->stackedWidget_, adwidgets::create_widget< RESTWnd >( "REST" ) ) ) {
        connect( document::instance(), &document::figshareReply, wnd, &RESTWnd::handleFigshareReply );
        connect( document::instance(), &document::downloadReply, wnd, &RESTWnd::handleDownloadReply );
    }

    if ( auto wnd = adwidgets::add_widget( impl_->stackedWidget_, adwidgets::create_widget< CSVWnd >( "CSVWnd" ) ) ) {
        connect( document::instance(), &document::csvReply, wnd, &CSVWnd::handleCSVReply );
    }

    if ( Core::MiniSplitter * splitter = new Core::MiniSplitter ) {
        splitter->addWidget( impl_->stackedWidget_ );
        splitter->setStretchFactor( 0, 1 );
        splitter->setStretchFactor( 1, 0 );

        if ( QVBoxLayout * centralLayout = new QVBoxLayout( centralWidget ) ) {
            centralLayout->setContentsMargins( {} );
            centralLayout->setSpacing( 0 );

            centralLayout->addWidget( toolBar1 );
            centralLayout->addWidget( splitter );
            centralLayout->addWidget( toolBar2 );
        }
    }

	createDockWidgets();
	setSimpleDockWidgetArrangement();

	return this; // mainWindowSplitter;
}

Utils::StyledBar *
MainWindow::createTopStyledBar()
{
    if ( auto toolBar = new Utils::StyledBar ) {
        toolBar->setProperty( "topBorder", true );
        toolBar->setObjectName( "toolBar1" );
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
        toolBarLayout->setContentsMargins( {} );
        toolBarLayout->setSpacing( 0 );

        if ( auto am = Core::ActionManager::instance() ) {
            Core::Context context( ( Utils::Id( "figshare.MainWindow" ) ) );

            if ( auto p = new QAction( tr("REST"), this ) ) {
                connect( p, &QAction::triggered, [this](){ impl_->stackedWidget_->setCurrentIndex( 0 ); } );
                am->registerAction( p, "figshare.REST", context );
                toolBarLayout->addWidget( ToolButton()( p, QString( "wnd.%1" ).arg( 0 ) ) );
            }
            if ( auto p = new QAction( tr("DATA"), this ) ) {
                connect( p, &QAction::triggered, [this](){ impl_->stackedWidget_->setCurrentIndex( 1 ); } );
                am->registerAction( p, "figshare.CSV", context );
                toolBarLayout->addWidget( ToolButton()( p, QString( "wnd.%1" ).arg( 1 ) ) );
            }
            //-- separator --
            toolBarLayout->addWidget( new Utils::StyledSeparator );
            toolBarLayout->addItem( new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum ) );
        }

        return toolBar;
    }
    return nullptr;
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

        toolBarLayout->addWidget( new Utils::StyledSeparator );
        toolBarLayout->addItem( new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum) );
    }
    return toolBar;
}

void
MainWindow::createDockWidgets()
{
    if ( auto w = new adwidgets::JSTRestForm( this ) ) {
        createDockWidget( w, "JST", "JST" );
        connect( w, &adwidgets::JSTRestForm::apply, document::instance(), &document::JSTREST );
        connect( w, &adwidgets::JSTRestForm::apply, [&](const QByteArray& ){ impl_->stackedWidget_->setCurrentIndex(0); } );
    }
    if ( auto w = new adwidgets::FigshareRESTForm( this ) ) {
        createDockWidget( w, "figshare", "figshare" );
        connect( w, &adwidgets::FigshareRESTForm::apply, document::instance(), &document::figshareREST );
        connect( w, &adwidgets::FigshareRESTForm::apply, [&](const QByteArray& ){ impl_->stackedWidget_->setCurrentIndex(0); } );
    }
}
