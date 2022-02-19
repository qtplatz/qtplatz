/**************************************************************************
** Copyright (C) 2010-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2022 MS-Cheminformatics LLC
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

#include "document.hpp"
#include "mainwindow.hpp"
#include "metidwidget.hpp"
#include "moltablewnd.hpp"
#include "msspectrawnd.hpp"
#include "mspeaktree.hpp"
#include "sqleditform.hpp"
#include "peaklist.hpp"
#include "sdfimport.hpp"
#include <adlog/logger.hpp>
#include <adcontrols/metidmethod.hpp>
#include <adportable/configuration.hpp>
#include <adportable/debug.hpp>
#include <adportable/profile.hpp>
#include <adportable/utf.hpp>
#include <adprocessor/processmediator.hpp>
#include <adprot/digestedpeptides.hpp>
#include <adprot/peptide.hpp>
#include <adprot/peptides.hpp>
#include <adutils/adfile.hpp>
#include <qtwrapper/trackingenabled.hpp>
#include <extensionsystem/pluginmanager.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/imode.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/documentmanager.h>
#include <coreplugin/findplaceholder.h>
#include <coreplugin/rightpane.h>
#include <coreplugin/minisplitter.h>
#include <coreplugin/outputpane.h>
#include <coreplugin/navigationwidget.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/icore.h>
#include <utils/styledbar.h>

#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QDockWidget>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QResizeEvent>
#include <QStackedWidget>
#include <QTabWidget>
#include <QTextEdit>
#include <QToolButton>
#include <QTreeView>
#include <QVBoxLayout>
#include <QVariant>
#include <QtGui/QIcon>
#include <boost/filesystem.hpp>
#include <boost/json.hpp>
#include <fstream>
#include <functional>
#include <tuple>
#include <utility>

namespace lipidid {

    enum {
        idSelSpectra
        , idSelMols
    };

    class MainWindow::impl {
        MainWindow * this_;
    public:
        ~impl() {}
        impl( MainWindow * p ) : this_( p )
                               , stackWidget_( 0 ) {
        }
        QStackedWidget * stackWidget_;
        Utils::StyledBar * createTopStyledToolbar();
        Utils::StyledBar * createMidStyledToolbar();

        static QToolButton * toolButton( QAction * action, const QString& objectName = {})  {
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

        static QToolButton *
        toolButton( const char * id ) {
            return toolButton( Core::ActionManager::instance()->command( id )->action() );
        }
        static void createDockWidgets( MainWindow * );
        void setup_menu_actions();
    };

}

namespace {

    using lipidid::MainWindow;

    QDockWidget * createDockWidget( MainWindow *, QWidget *, const QString&, const QString& );

    template< typename T > auto dock_create( MainWindow * p, const QString& name, const QString& page ) {
        auto w = new T( p );
        createDockWidget( p, w, name, page );
        return w;
    }
}


using lipidid::MainWindow;

MainWindow::~MainWindow()
{
}

MainWindow::MainWindow( QWidget *parent )
    : Utils::FancyMainWindow(parent)
    , impl_( std::make_unique< impl >( this ) )
{
}

MainWindow *
MainWindow::instance()
{
    static MainWindow __instance;
    return &__instance;
}

void
MainWindow::activateLayout()
{
}

namespace {
    template<typename T> struct dockBroker {
        static void onInitialUpdate(MainWindow * p) {
            if ( auto w = p->findChild<T *>() ) {
                w->onInitialUpdate();
            }
        }
    };

    template<typename... Args> void onInitialUpdate( MainWindow * p ){
        (( dockBroker<Args>::onInitialUpdate( p )), ...);
    }
}

void
MainWindow::OnInitialUpdate()
{
    impl::createDockWidgets( this );
    MainWindow::setSimpleDockWidgetArrangement();

    connect( document::instance(), &document::onConnectionChanged, [this]{
        if ( auto table = findChild< MolTableWnd * >() ) {
            table->setQuery("SELECT * from mols");
            // table->setQuery(
            //     "SELECT t1.id,svg,synonym,formula,mass,csid,smiles,InChI,InChiKey,SystematicName"
            //     " FROM mols t1 LEFT OUTER JOIN synonyms t2 on t1.id = t2.id" );
        }
    } );

    onInitialUpdate< MetIdWidget >( this );
    onInitialUpdate< MSPeakTree >( this );

    document::instance()->initialSetup();
}

void
MainWindow::OnFinalClose()
{
    document::instance()->finalClose();
}

void
MainWindow::hideDock( bool hide )
{
    ADDEBUG() << "-------------- hideDock -----------------";
    for ( auto& w :  dockWidgets() ) {
        if ( hide )
            w->hide();
        else
            w->show();
    }
}

void
MainWindow::importSDFile( bool flag )
{
    ADDEBUG() << "-------------- import sdfile -----------------";
    if ( auto sdfimport = std::make_shared< SDFileImport >( this ) )
        sdfimport->import();
    ADDEBUG() << "-------------- import sdfile done -----------------";
}

QWidget *
MainWindow::createContents( Core::IMode * mode )
{
    setTabPosition( Qt::AllDockWidgetAreas, QTabWidget::South );
    setDocumentMode( true );
    setDockNestingEnabled( true );

    Utils::StyledBar * toolBar1 = impl_->createTopStyledToolbar();
    Utils::StyledBar * toolBar2 = impl_->createMidStyledToolbar();

    //---------- central widget ------------
    QWidget * centralWidget = new QWidget;
    setCentralWidget( centralWidget );

    Core::MiniSplitter * splitter3 = new Core::MiniSplitter;
    if ( splitter3 ) {

        impl_->stackWidget_ = new QStackedWidget;
        splitter3->addWidget( impl_->stackWidget_ );

        connect( impl_->stackWidget_, &QStackedWidget::currentChanged, this, [&]( int idx ){
                auto list = findChildren<QToolButton *>( QRegExp( "wnd\\.[0-9]+" ) );
                for ( auto btn : list ) {
                    if ( btn->objectName() == QString( "wnd.%1" ).arg( QString::number( idx ) ) ) {
                        btn->setStyleSheet( QString( "color: ivory; border: 2px; border-color: darkGray; border-style: inset;" ) );
                    } else {
                        btn->setStyleSheet( QString( "color: lightGray; border: 2px; border-color: gray; border-style: groove" ) );
                    }
                }
            });

        if ( auto pWnd = new MSSpectraWnd ) {
            pWnd->setWindowTitle( "Spectra" );
            impl_->stackWidget_->addWidget( pWnd );
            connect( document::instance(), &document::dataChanged, [=](auto& f){ pWnd->handleDataChanged(f);} );
            connect( document::instance(), &document::idCompleted, pWnd, &MSSpectraWnd::handleIdCompleted );
        }

        if ( auto pWnd = new MolTableWnd ) {
            pWnd->setWindowTitle( "Mols" );
            impl_->stackWidget_->addWidget( pWnd );
        }
    }

    QBoxLayout * toolBarAddingLayout = new QVBoxLayout( centralWidget );
    toolBarAddingLayout->setMargin(0);
    toolBarAddingLayout->setSpacing(0);
    toolBarAddingLayout->addWidget( toolBar1 );  // top most toolbar
    toolBarAddingLayout->addWidget( splitter3 ); // Spectra|chrmatogram pane
    toolBarAddingLayout->addWidget( toolBar2 );  // middle toolbar

    // Right-side window with editor, output etc.
    Core::MiniSplitter * mainWindowSplitter = new Core::MiniSplitter;
    QWidget * outputPane = new Core::OutputPanePlaceHolder( mode, mainWindowSplitter );
    outputPane->setObjectName( QLatin1String( "OutputPanePlaceHolder" ) );
    mainWindowSplitter->addWidget( this );
    mainWindowSplitter->addWidget( outputPane );
    mainWindowSplitter->setStretchFactor( 0, 10 );
    mainWindowSplitter->setStretchFactor( 1, 0 );
    mainWindowSplitter->setOrientation( Qt::Vertical );

#if 0
    // Navigation and right-side window
    Core::MiniSplitter * splitter = new Core::MiniSplitter;
    splitter->addWidget( new Core::NavigationWidgetPlaceHolder( mode ) );
    splitter->addWidget( mainWindowSplitter );
    splitter->setStretchFactor( 0, 0 );
    splitter->setStretchFactor( 1, 1 );
    splitter->setObjectName( QLatin1String( "ModeWidget" ) );
	return splitter;
#endif
    return mainWindowSplitter;
}

Utils::StyledBar *
MainWindow::impl::createTopStyledToolbar()
{
    Utils::StyledBar * toolBar = new Utils::StyledBar;
    if ( toolBar ) {
        toolBar->setProperty( "topBorder", true );
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
        toolBarLayout->setMargin( 0 );
        toolBarLayout->setSpacing( 0 );
        if ( auto am = Core::ActionManager::instance() ) {
            Core::Context context( ( Core::Id( "lipidid.MainWindow" ) ) );

            if ( auto p = new QAction( tr("Spectra"), this_ ) ) {
                connect( p, &QAction::triggered, [=](){ stackWidget_->setCurrentIndex( idSelSpectra ); } );
                am->registerAction( p, "lipidid.selSpectra", context );
                toolBarLayout->addWidget( toolButton( p, QString( "wnd.%1" ).arg( idSelSpectra ) ) );
            }

            if ( auto p = new QAction( tr("Mols"), this_ ) ) {
                connect( p, &QAction::triggered, [=](){ stackWidget_->setCurrentIndex( idSelMols ); } );
                am->registerAction( p, "lipidid.selMols", context );
                toolBarLayout->addWidget( toolButton( p, QString( "wnd.%1" ).arg( idSelMols ) ) );
            }

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

Utils::StyledBar *
MainWindow::impl::createMidStyledToolbar()
{
    if ( Utils::StyledBar * toolBar = new Utils::StyledBar ) {

        toolBar->setProperty( "topBorder", true );
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
        toolBarLayout->setMargin(0);
        toolBarLayout->setSpacing(0);
        Core::ActionManager * am = Core::ActionManager::instance();
        if ( am ) {
            //----------
            toolBarLayout->addWidget( new Utils::StyledSeparator );
            //----------
            toolBarLayout->addItem( new QSpacerItem(32, 20, QSizePolicy::Minimum, QSizePolicy::Minimum) );
            toolBarLayout->addWidget( new Utils::StyledSeparator );
            if ( auto label = new QLabel ) {
                label->setText( tr("Control Method:" ) );
                toolBarLayout->addWidget( label );
            }
            toolBarLayout->addItem( new QSpacerItem(16, 20, QSizePolicy::Expanding, QSizePolicy::Minimum) );
            toolBarLayout->addWidget( toolButton( am->command( Constants::HIDE_DOCK )->action() ) );
        }
		return toolBar;
    }
    return 0;
}

void
MainWindow::impl::createDockWidgets( MainWindow * pThis )
{
    if ( auto widget = dock_create< PeakList >( pThis, "MS Peaks", "MS_Peaks" ) ) {
        QObject::connect( document::instance(), &document::dataChanged, widget, &PeakList::handleDataChanged );
    }
    if ( auto widget = dock_create< MetIdWidget >( pThis, "Adducts/Losses", "Adducts" ) ) {
        QObject::connect( widget, &MetIdWidget::triggered, [=]{ document::instance()->find_all( widget->getContents() ); } );
    }
    if ( auto widget = dock_create< SqlEditForm >( pThis, "SQL", "SqlEditForm" ) ) {
        if ( auto table = pThis->findChild< MolTableWnd * >() ) {
            QObject::connect( widget, &SqlEditForm::triggerQuery, table, &MolTableWnd::setQuery );
        }
    }
    if ( auto widget = dock_create< MSPeakTree >( pThis, "Peaks", "Peaks" ) ) {
        QObject::connect( document::instance(), &document::dataChanged, widget, &MSPeakTree::handleDataChanged );
        QObject::connect( document::instance(), &document::idCompleted, widget, &MSPeakTree::handleIdCompleted );
    }
}

namespace {

    QDockWidget *
    createDockWidget( MainWindow * pThis
                      , QWidget * widget
                      , const QString& title
                      , const QString& pageName )
    {
        if ( widget->windowTitle().isEmpty() ) // avoid QTC_CHECK warning on console
            widget->setWindowTitle( title );

        if ( widget->objectName().isEmpty() )
            widget->setObjectName( pageName );

        QDockWidget * dockWidget = pThis->addDockForWidget( widget );
        dockWidget->setObjectName( pageName.isEmpty() ? widget->objectName() : pageName );

        if ( title.isEmpty() )
            dockWidget->setWindowTitle( widget->objectName() );
        else
            dockWidget->setWindowTitle( title );

        pThis->addDockWidget( Qt::BottomDockWidgetArea, dockWidget );

        return dockWidget;
    }

}

void
MainWindow::initializeActions( Core::IMode * mode )
{
    if ( auto am = Core::ActionManager::instance() ) {
        QIcon icon;
        icon.addPixmap( QPixmap( Constants::ICON_DOCKHIDE ), QIcon::Normal, QIcon::Off );
        icon.addPixmap( QPixmap( Constants::ICON_DOCKSHOW ), QIcon::Normal, QIcon::On );
        auto * action = new QAction( icon, QObject::tr( "Hide dock" ), this );
        action->setCheckable( true );
        am->registerAction( action, Constants::HIDE_DOCK, mode->context() );
        connect( action, &QAction::triggered, MainWindow::instance(), &MainWindow::hideDock );
    }
    if ( auto am = Core::ActionManager::instance() ) {
        auto * action = new QAction( QObject::tr( "Import SDFile" ), this );
        am->registerAction( action, Constants::SDF_IMPORT, mode->context() );
        connect( action, &QAction::triggered, MainWindow::instance(), &MainWindow::importSDFile );
    }
    impl_->setup_menu_actions();
}

void
MainWindow::impl::setup_menu_actions()
{
	if ( auto * am = Core::ActionManager::instance() ) {
        if ( Core::ActionContainer * menu = am->createMenu( "lipidid.window.menu" ) ) {
            menu->menu()->setTitle( tr("LipidId") );
            menu->addAction( am->command( Constants::HIDE_DOCK ) );
            am->actionContainer( Core::Constants::M_WINDOW )->addMenu( menu );
        }

        if ( Core::ActionContainer * menu = am->createMenu( "lipidid.file.menu" ) ) {
            menu->menu()->setTitle( tr("LipidId") );
            menu->addAction( am->command( Constants::SDF_IMPORT ) );
            am->actionContainer( Core::Constants::M_FILE )->addMenu( menu );
        }
    }
}

void
MainWindow::setSimpleDockWidgetArrangement()
{
    qtwrapper::TrackingEnabled<Utils::FancyMainWindow> x( *this );

    QList< QDockWidget *> widgets = dockWidgets();

    for ( auto widget: widgets ) {
        widget->setFloating( false );
        removeDockWidget( widget );
    }

    size_t npos = 0;
    for ( auto widget: widgets ) {
        addDockWidget( Qt::BottomDockWidgetArea, widget );
        widget->show();
        if ( npos++ >= 2 )
            tabifyDockWidget( widgets[1], widget );
    }
	widgets[1]->raise();

    update();
}
