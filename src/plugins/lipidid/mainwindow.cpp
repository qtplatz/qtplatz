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

#include "mainwindow.hpp"
#include "document.hpp"
#include <adwidgets/mspeaktree.hpp>
#include <adlog/logger.hpp>
#include <adportable/configuration.hpp>
#include <adportable/debug.hpp>
#include <adportable/profile.hpp>
#include <adportable/utf.hpp>
#include <adprocessor/processmediator.hpp>
#include <adprot/digestedpeptides.hpp>
#include <adprot/peptide.hpp>
#include <adprot/peptides.hpp>
#include <adutils/adfile.hpp>
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
#include <QJsonObject>
#include <QJsonDocument>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QResizeEvent>
#include <QStandardPaths>
#include <QTabWidget>
#include <QTextEdit>
#include <QToolButton>
#include <QTreeView>
#include <QVBoxLayout>
#include <QVariant>
#include <QtGui/QIcon>
#include <qdebug.h>
#include <qstackedwidget.h>
#include <boost/json.hpp>
#include <boost/filesystem.hpp>
#include <functional>
#include <fstream>

namespace lipidid {

    struct wnd_set_title : public boost::static_visitor < QWidget * > {
        const QString& text_;
        wnd_set_title( const QString& t ) : text_( t ) {}
        template<class T> QWidget * operator () ( T* wnd ) const { wnd->setWindowTitle( text_ ); return wnd; }
    };

    class MainWindow::impl {
        MainWindow * this_;
    public:
        ~impl() {}
        impl( MainWindow * p ) : this_( p )
                               , topBar_(0)
                               , midBar_(0) {
        }
        Utils::StyledBar * topBar_;
        Utils::StyledBar * midBar_;
        static Utils::StyledBar * createTopStyledToolbar();
        static Utils::StyledBar * createMidStyledToolbar();

        static QToolButton * toolButton( QAction * action )  {
            QToolButton * button = new QToolButton;
            if ( button )
                button->setDefaultAction( action );
            return button;
        }

        static QToolButton *
        toolButton( const char * id ) {
            return toolButton( Core::ActionManager::instance()->command( id )->action() );
        }
    };
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


void
MainWindow::OnInitialUpdate()
{
}

void
MainWindow::OnFinalClose()
{
}

void
MainWindow::hideDock( bool hide )
{
    for ( auto& w :  dockWidgets() ) {
        if ( hide )
            w->hide();
        else
            w->show();
    }
}

QWidget *
MainWindow::createContents( Core::IMode * mode )
{
    setTabPosition( Qt::AllDockWidgetAreas, QTabWidget::North );
    setDocumentMode( true );
    setDockNestingEnabled( true );

    QBoxLayout * editorHolderLayout = new QVBoxLayout;
	editorHolderLayout->setMargin( 0 );
	editorHolderLayout->setSpacing( 0 );

    if ( QWidget * editorWidget = new QWidget ) {

        editorWidget->setLayout( editorHolderLayout );

        editorHolderLayout->addWidget( impl_->createTopStyledToolbar() );
        if ( auto wnd = new QWidget() ) { // WaveformWnd() ) {
            editorHolderLayout->addWidget( wnd );
        }

        //---------- central widget ------------
        if ( QWidget * centralWidget = new QWidget ) {
            setCentralWidget( centralWidget );

            QVBoxLayout * centralLayout = new QVBoxLayout( centralWidget );
            centralWidget->setLayout( centralLayout );
            centralLayout->setMargin( 0 );
            centralLayout->setSpacing( 0 );
            // ----------------------------------------------------
            centralLayout->addWidget( editorWidget ); // [ToolBar + WaveformWnd]
            // ----------------------------------------------------
            ADDEBUG() << "## " << __FUNCTION__ << " ##"; // ok
            //centralLayout->setStretch( 0, 1 );
            //centralLayout->setStretch( 1, 0 );
            // ----------------- mid tool bar -------------------
            centralLayout->addWidget( impl_->createMidStyledToolbar() );      // [Middle toolbar]
            ADDEBUG() << "## " << __FUNCTION__ << " ##"; // ok
        }
    }
    ADDEBUG() << "## " << __FUNCTION__ << " ##";
    if ( Core::MiniSplitter * mainWindowSplitter = new Core::MiniSplitter ) {

        QWidget * outputPane = new Core::OutputPanePlaceHolder( mode, mainWindowSplitter );
        outputPane->setObjectName( QLatin1String( "SequenceOutputPanePlaceHolder" ) );

        mainWindowSplitter->addWidget( this );        // [Central Window]
        mainWindowSplitter->addWidget( outputPane );  // [Output (log) Window]

        mainWindowSplitter->setStretchFactor( 0, 9 );
        mainWindowSplitter->setStretchFactor( 1, 1 );
        mainWindowSplitter->setOrientation( Qt::Vertical );

        // Split Navigation and Application window
        Core::MiniSplitter * splitter = new Core::MiniSplitter;               // entier this view
        if ( splitter ) {
            splitter->addWidget( mainWindowSplitter );                            // *this + ontput
            splitter->setOrientation( Qt::Horizontal );
            splitter->setObjectName( QLatin1String( "SequenceModeWidget" ) );
        }
        // createDockWidgets();
        return splitter;
    }
    return 0;
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
    ADDEBUG() << "## " << __FUNCTION__ << " ##";
    if ( Utils::StyledBar * toolBar = new Utils::StyledBar ) {

        toolBar->setProperty( "topBorder", true );
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
        toolBarLayout->setMargin(0);
        toolBarLayout->setSpacing(0);
        Core::ActionManager * am = Core::ActionManager::instance();
        ADDEBUG() << "## " << __FUNCTION__ << " ## am=" << am;
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
            // toolBarLayout->addWidget( toolButton( am->command( Constants::HIDE_DOCK )->action() ) );
        }
		return toolBar;
    }
    return 0;
}
