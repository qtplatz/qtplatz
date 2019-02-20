/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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
#include "peptidemode.hpp"
#include "peptideconstants.hpp"
#include "proteinwnd.hpp"
#include <qtwrapper/trackingenabled.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adportable/profile.hpp>
#include <adlog/logger.hpp>
#include <adprot/protfile.hpp>
#include <adprot/protease.hpp>
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

namespace peptide {
    MainWindow * MainWindow::instance_ = 0;
}

using namespace peptide;

MainWindow::MainWindow(QWidget *parent) : Utils::FancyMainWindow(parent)
                                        , topLineEdit_( new QLineEdit )
                                        , protease_( std::make_shared< adprot::protease >( "trypsin" ) )
                                        , formulaParser_( std::make_shared< adcontrols::ChemicalFormula >() )
{
    std::fill( actions_.begin(), actions_.end(), static_cast<QAction *>(0) );
    instance_ = this;
}

MainWindow::~MainWindow()
{
}

MainWindow *
MainWindow::instance()
{
    return instance_;
}

QWidget *
MainWindow::createContents( Core::IMode * mode )
{
    setTabPosition( Qt::AllDockWidgetAreas, QTabWidget::East );
    setDocumentMode( true );
    setDockNestingEnabled( true );

    QBoxLayout * editorHolderLayout = new QVBoxLayout;
	editorHolderLayout->setMargin( 0 );
	editorHolderLayout->setSpacing( 0 );

    if ( QWidget * editorWidget = new QWidget ) {

        editorWidget->setLayout( editorHolderLayout );
        editorHolderLayout->addWidget( new ProteinWnd() );

        Utils::StyledBar * toolBar1 = createTopStyledBar();
        Utils::StyledBar * toolBar2 = createMidStyledBar();

        //---------- central widget ------------
        QWidget * centralWidget = new QWidget;
        if ( centralWidget ) {
            setCentralWidget( centralWidget );

            QVBoxLayout * centralLayout = new QVBoxLayout( centralWidget );
            centralWidget->setLayout( centralLayout );
            centralLayout->setMargin( 0 );
            centralLayout->setSpacing( 0 );
            // ----------------- top tool bar -------------------
            centralLayout->addWidget( toolBar1 );              // [1]
            // ----------------------------------------------------

            centralLayout->addWidget( editorWidget ); // [0]
            centralLayout->setStretch( 0, 1 );
            centralLayout->setStretch( 1, 0 );

            // ----------------- mid tool bar -------------------
            centralLayout->addWidget( toolBar2 );              // [1]
        }
    }

	// Right-side window with editor, output etc.
	Core::MiniSplitter * mainWindowSplitter = new Core::MiniSplitter;
    if ( mainWindowSplitter ) {
        QWidget * outputPane = new Core::OutputPanePlaceHolder( mode, mainWindowSplitter );
        outputPane->setObjectName( QLatin1String( "SequenceOutputPanePlaceHolder" ) );
        mainWindowSplitter->addWidget( this );
        mainWindowSplitter->addWidget( outputPane );
        mainWindowSplitter->setStretchFactor( 0, 10 );
        mainWindowSplitter->setStretchFactor( 1, 0 );
        mainWindowSplitter->setOrientation( Qt::Vertical );
    }

	// Navigation and right-side window
	Core::MiniSplitter * splitter = new Core::MiniSplitter;               // entier this view
    if ( splitter ) {
        splitter->addWidget( new Core::NavigationWidgetPlaceHolder( mode ) ); // navegate
        splitter->addWidget( mainWindowSplitter );                            // *this + ontput
        splitter->setStretchFactor( 0, 0 );
        splitter->setStretchFactor( 1, 1 );
        splitter->setObjectName( QLatin1String( "SequenceModeWidget" ) );
    }

    createDockWidgets();

	return splitter;
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
        // Core::ActionManager * am = Core::ICore::instance()->actionManager();
        if ( auto am = Core::ActionManager::instance() ) {
            // [file open] button
            toolBarLayout->addWidget(toolButton(am->command(Constants::FILE_OPEN)->action()));
            //-- separator --
            toolBarLayout->addWidget( new Utils::StyledSeparator );
            //---
            toolBarLayout->addWidget( topLineEdit_.get() );
        }
        toolBarLayout->addWidget( new Utils::StyledSeparator );
        toolBarLayout->addItem( new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum) );
    }
    return toolBar;
}

Utils::StyledBar *
MainWindow::createMidStyledBar()
{
    if ( Utils::StyledBar * toolBar = new Utils::StyledBar ) {

        toolBar->setProperty( "topBorder", true );
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
        toolBarLayout->setMargin(0);
        toolBarLayout->setSpacing(0);
        // Core::ActionManager * am = Core::ICore::instance()->actionManager();
        if( auto am = Core::ActionManager::instance() ) {
            (void)am;
            // print, method file open & save buttons
            //toolBarLayout->addWidget(toolButton(am->command(Constants::PRINT_CURRENT_VIEW)->action()));
            //toolBarLayout->addWidget(toolButton(am->command(Constants::METHOD_OPEN)->action()));
            //toolBarLayout->addWidget(toolButton(am->command(Constants::FILE_OPEN)->action()));
            //----------
            toolBarLayout->addWidget( new Utils::StyledSeparator );
            //----------
            Core::Context context( (Core::Id( Core::Constants::C_GLOBAL )) );
            // context << Core::Constants::C_GLOBAL_ID;

            // QComboBox * features = new QComboBox;
            // features->addItem( "Centroid" );
            // features->addItem( "Isotope" );
            // features->addItem( "Calibration" );
            // features->addItem( "Find peaks" );
            // toolBarLayout->addWidget( features );

            //connect( features, SIGNAL( currentIndexChanged(int) ), this, SLOT( handleFeatureSelected(int) ) );
            //connect( features, SIGNAL( activated(int) ), this, SLOT( handleFeatureActivated(int) ) );

            //----------
            toolBarLayout->addWidget( new Utils::StyledSeparator );

            toolBarLayout->addItem( new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum) );
        }
		return toolBar;
    }
    return 0;
}

void
MainWindow::onInitialUpdate()
{
    setSimpleDockWidgetArrangement();
    setDemoData();
    if ( auto wnd = findChild< ProteinWnd * >() ) {
        //wnd->setStyleSheet( "* { font-size: 9pt; background-color: cyan;}" );

        wnd->setStyleSheet( "QHeaderView::section {"
                            "  background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1"
                            "    ,stop:0 #616161, stop: 0.5 #505050"
                            "    ,stop: 0.6 #434343, stop:1 #656565 );"
                            "  color: white;"
                            "  padding-left: 4px;"
                            "  border: 1px solid #6c6c6c;"
#if !defined Q_OS_MAC
                            "  font-size: 9pt;"
#endif
                            "}"
                            // "QHeaderView::section:checked {"
                            // "  background-color: red;"
                            // "  font-size: 9pt;"
                            // "}"
#if !defined Q_OS_MAC
                            "QTableView {"
                            "  font-size: 9pt;"
                            "}"
#endif
            );
    }
}

void
MainWindow::createDockWidgets()
{
    createDockWidget( new QTextEdit(), "Sequence" );
    // if ( QWidget * w = new DropTargetForm( this ) ) {
    //     connect( w, SIGNAL( dropped( const QList<QString>& ) ), this, SLOT( handleDropped( const QList<QString>& ) ) );
    //     createDockWidget( w, "Drop Target" );
    // }
}

QDockWidget *
MainWindow::createDockWidget( QWidget * widget, const QString& title )
{
    if ( widget->windowTitle().isEmpty() ) // avoid QTC_CHECK warning on console
        widget->setWindowTitle( title );
    if ( widget->objectName().isEmpty() )
        widget->setObjectName( title );

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
MainWindow::setSimpleDockWidgetArrangement()
{
    qtwrapper::TrackingEnabled< Utils::FancyMainWindow > x( *this );

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
	// widgets[1]->raise();

    // QDockWidget * toolBarDock = toolBarDockWidget();
    // if ( toolBarDock )
    //     toolBarDock->show();
    update();
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
    //Core::ActionManager * mgr = Core::ICore::instance()->actionManager();
    return toolButton( Core::ActionManager::instance()->command( id )->action() );
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
    actions_[ idActFileOpen ] = createAction( Constants::ICON_FILE_OPEN, tr("Open protain file..."), this );
    connect( actions_[ idActFileOpen ], SIGNAL( triggered() ), this, SLOT( actFileOpen() ) );

    Core::Context gc( (Core::Id( Core::Constants::C_GLOBAL )) );

    if ( Core::ActionManager * am = Core::ActionManager::instance() ) {

        Core::ActionContainer * menu = am->createMenu( Constants::MENU_ID ); // Menu ID
        menu->menu()->setTitle( tr("Peptide") );

        Core::Command * cmd = 0;

        cmd = am->registerAction( actions_[ idActFileOpen ], Constants::FILE_OPEN, gc );
        menu->addAction( cmd );

        am->actionContainer( Core::Constants::M_TOOLS )->addMenu( menu );
    }
}

void
MainWindow::setDemoData()
{
    topLineEdit_->setText( tr( "select a file contains protein sequences by clicking file open icon..." ) );
    adprot::protein proteins [] = { { "Bovin serum albumin" // protein
                                      ,""                   // gene
                                      ,""                   // organism
                                      ,""                   // url
                                      ," MKWVTFISLL LLFSSAYSRG VFRRDTHKSE IAHRFKDLGE EHFKGLVLIA FSQYLQQCPF" \
                                      " DEHVKLVNEL TEFAKTCVAD ESHAGCEKSL HTLFGDELCK VASLRETYGD MADCCEKQEP" \
                                      " ERNECFLSHK DDSPDLPKLK PDPNTLCDEF KADEKKFWGK YLYEIARRHP YFYAPELLYY" \
                                      " ANKYNGVFQE CCQAEDKGAC LLPKIETMRE KVLASSARQR LRCASIQKFG ERALKAWSVA" \
                                      " RLSQKFPKAE FVEVTKLVTD LTKVHKECCH GDLLECADDR ADLAKYICDN QDTISSKLKE" \
                                      " CCDKPLLEKS HCIAEVEKDA IPENLPPLTA DFAEDKDVCK NYQEAKDAFL GSFLYEYSRR" \
                                      " HPEYAVSVLL RLAKEYEATL EECCAKDDPH ACYSTVFDKL KHLVDEPQNL IKQNCDQFEK" \
                                      " LGEYGFQNAL IVRYTRKVPQ VSTPTLVEVS RSLGKVGTRC CTKPESERMP CTEDYLSLIL" \
                                      " NRLCVLHEKT PVSEKVTKCC TESLVNRRPC FSALTPDETY VPKAFDEKLF TFHADICTLP" \
                                      " DTEKQIKKQT ALVELLKHKP KATEEQLKTV MENFVAFVDK CCAADDKEAC FAVEGPKLVV" \
                                      " STQTALA"
        }
                                    , {  "Cytochrome b-c1 complex subunit Rieske, mitochondrial"
                                         , "UQCRSS1"
                                         , "Mouse musculus (Bovine)"
                                         , "https://www.uniprot.org/uniprot/P13272"
                                         , "MLSVAARSGP FAPVLSATSR GVAGALRPLL QGAVPAASEP PVLDVKRPFL"
                                         "CRESLSGQAA ARPLVATVGL NVPASVRFSH TDVKVPDFSD YRRAEVLDST"
                                         "KSSKESSEAR KGFSYLVTAT TTVGVAYAAK NVVSQFVSSM SASADVLAMS"
                                         "KIEIKLSDIP EGKNMAFKWR GKPLFVRHRT KKEIDQEAAV EVSQLRDPQH"
                                         "DLDRVKKPEW VILIGVCTHL GCVPIANAGD FGGYYCPCHG SHYDASGRIR"
                                         "KGPAPLNLEV PAYEFTSDDV VVVG"
        }
                                    , { "Cytochrome c"
                                        , "CYCS"
                                        , "Equus caballus (Horse)"
                                        , "https://www.uniprot.org/uniprot/P00004"
                                        , "MGDVEKGKKI FVQKCAQCHT VEKGGKHKTG PNLHGLFGRK TGQAPGFTYT "
                                        "DANKNKGITW KEETLMEYLE NPKKYIPGTK MIFAGIKKKT EREDLIAYLK "
                                        "KATNE             "
        }
                                    , { ">sp|P62894|CYC_BOVIN Cytochrome c OS=Bos taurus OX=9913 GN=CYCS PE=1 SV=2"
                                        , "MGDVEKGKKIFVQKCAQCHTVEKGGKHKTGPNLHGLFGRKTGQAPGFSYTDANKNKGITW"
                                        "GEETLMEYLENPKKYIPGTKMIFAGIKKKGEREDLIAYLKKATNE" }
                                    
    };


	protfile_ = std::make_shared< adprot::protfile >( "demo data" );
    for ( const auto& prot: proteins )
        *protfile_ << prot;

    if ( auto wnd = findChild< ProteinWnd * >() )
        wnd->setData( *protfile_ );
}

void
MainWindow::actFileOpen()
{
    boost::filesystem::path datapath( adportable::profile::user_data_dir<char>() );
    datapath /= "data";

    QString name
        = QFileDialog::getOpenFileName( this
                                        , tr("Open protein definition file" )
										, datapath.string().c_str()
                                        , tr("Protain sequence files(*.fas)") );
    if ( ! name.isEmpty() ) {
        topLineEdit_->setText( name );

        auto file = std::make_shared< adprot::protfile >( name.toStdString() );
        if ( *file ) {
            protfile_ = file;
            if ( auto wnd = findChild< ProteinWnd * >() )
                wnd->setData( *protfile_ );
        }

	}
}

const std::shared_ptr< adprot::protfile >&
MainWindow::get_protfile() const
{
    return protfile_;
}

const std::shared_ptr< adprot::protease >&
MainWindow::get_protease() const
{
    return protease_;
}

const std::shared_ptr< adcontrols::ChemicalFormula >&
MainWindow::getChemicalFormula() const
{
    return formulaParser_;
}
