/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include "../lrpfile/lrpfile.hpp"
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adplot/chromatogramwidget.hpp>
#include <adplot/spectrumwidget.hpp>
#include <adportable/profile.hpp>
#include <qtwrapper/settings.hpp>
#include <QAction>
#include <QBoxLayout>
#include <QDockWidget>
#include <QFileDialog>
#include <QLabel>
#include <QMenuBar>
#include <QMenu>
#include <QMessageBox>
#include <QSettings>
#include <QSplitter>
#include <QStandardItemModel>
#include <QStatusBar>
#include <QTextEdit>
#include <QTreeView>
#include <QToolBar>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <sstream>
#include <iostream>

struct user_preference {
    static boost::filesystem::path path( QSettings * settings ) {
        boost::filesystem::path dir( settings->fileName().toStdWString() );
        return dir.remove_filename() / "lrpviewer";
    }
};


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
                                        , index_( 0 )
                                        , settings_( std::make_shared< QSettings >( QSettings::IniFormat, QSettings::UserScope
                                                                                    , QLatin1String( "lrpviewer" )
                                                                                    , QLatin1String( "lrpviewer" ) ) )
{
    QWidget * centralWidget = new QWidget( this );
    centralWidget->setObjectName( "centralWidget" );
    this->setCentralWidget( centralWidget );
    this->setStatusBar( new QStatusBar( this ) );
    this->setMenuBar( new QMenuBar( this ) );

	if ( QMenu * fileMenu = this->menuBar()->addMenu( tr("&File") ) ) {
		fileMenu->addAction( tr("Open TSS Pro .lrp file..."), this, SLOT( actFileOpen() ) );
	}
	if ( QMenu * viewMenu = this->menuBar()->addMenu( tr("&View") ) ) {
		viewMenu->addAction( tr("View next"), this, SLOT( actViewNext() ) );
	}

    QToolBar * toolBar = new QToolBar( this );
    do {
        toolBar->setProperty( "topBorder", true );
        QHBoxLayout * layout = new QHBoxLayout( toolBar );
        layout->setMargin( 0 );
        layout->setSpacing( 0 );
        layout->addWidget( new QLabel( "Status:" ) );
    } while(0);

    QVBoxLayout * toolBarLayout = new QVBoxLayout( centralWidget );
    toolBarLayout->setMargin( 0 );
    toolBarLayout->setSpacing(0);
    toolBarLayout->addWidget( new adplot::ChromatogramWidget );
    toolBarLayout->addWidget( new adplot::SpectrumWidget );
    toolBarLayout->addWidget( toolBar );

    this->addDockWidget( Qt::BottomDockWidgetArea, addDockForWidget( new QTextEdit ) );
    initialSetup();
}

MainWindow::~MainWindow()
{
}

QDockWidget *
MainWindow::addDockForWidget( QWidget * widget )
{
    QDockWidget *dockWidget = new QDockWidget(widget->windowTitle(), this);
    dockWidget->setObjectName(widget->windowTitle());
    dockWidget->setWidget(widget);
    updateDockWidget(dockWidget);
    return dockWidget;
}

void
MainWindow::updateDockWidget(QDockWidget *dockWidget)
{
    const QDockWidget::DockWidgetFeatures features =
        QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable;
    QWidget *titleBarWidget = dockWidget->titleBarWidget();
    if ( dockWidget->isFloating() && titleBarWidget) {
        delete titleBarWidget;
        titleBarWidget = 0;
    }
    dockWidget->setTitleBarWidget(titleBarWidget);
    dockWidget->setFeatures(features);
}

bool
MainWindow::Open( const std::string& filename )
{
    index_ = 0;
    boost::filesystem::path fpath( filename );
    if ( boost::filesystem::exists( fpath ) ) {
        size_t fsize = boost::filesystem::file_size( fpath );
        boost::filesystem::ifstream in( fpath, std::ios_base::binary );
        std::ostringstream o;
        o << fpath << std::endl;
        if ( ( spcfile_ = std::make_shared< shrader::lrpfile >( in, fsize ) ) ) {
            dumpspc( *spcfile_, o );
        }
        drawTIC( *spcfile_ );
        draw( 0 );
        return true;
    }
    return false;
}

void
MainWindow::dumpspc( const shrader::lrpfile& spc, std::ostream& o )
{
    std::ofstream out( "dump.txt" );
    spc.dump( out );
#if 0
    std::ostringstream out;
    if ( auto edit = findChild< QTextEdit * >() ) {
        edit->insertPlainText( QString::fromStdString( out.str() ) );
    }
#endif
}

void
MainWindow::actFileOpen()
{
    boost::filesystem::path datapath( adportable::profile::user_data_dir<char>() );
    
    QString name =
        QFileDialog::getOpenFileName( this
                                      , tr("Open TSS Pro .lrp file")
                                      , datapath.string().c_str()
                                      , tr("TSS Pro LRP Files(*.lrp)") );
    if ( !name.isEmpty() ) {
        if ( adplot::SpectrumWidget * w = findChild< adplot::SpectrumWidget * >() ) {
            fpath_ = name.toStdString();
            w->setTitle( fpath_ );

            if ( Open( name.toStdString() ) ) {
                std::ostringstream o;
                // o << spcfile_->source_instrument_description() << "\t" << spcfile_->date();
				// w->setFooter( o.str() );
                // if ( const galactic::spchdr * h = spcfile_->spchdr() ) {
                //     w->setAxisTitle( QwtPlot::xBottom, QwtText( spcfile_->axis_x_label() ) );
                //     w->setAxisTitle( QwtPlot::yLeft, QwtText( spcfile_->axis_y_label() ) );
                // }
            }
        }
    }
}

void
MainWindow::actViewNext()
{
    draw( index_ + 1 );
}

void
MainWindow::drawTIC( const shrader::lrpfile& lrp )
{
    std::vector< double > time, intens;
    if ( lrp.getTIC( time, intens ) ) {

        auto cptr = std::make_shared< adcontrols::Chromatogram >();

        cptr->resize( time.size() );
        cptr->setTimeArray( time.data() );
        cptr->setIntensityArray( intens.data() );

        if ( auto widget = findChild< adplot::ChromatogramWidget * >() )
            widget->setData( cptr );
    }
}

void
MainWindow::draw( size_t index )
{
    std::vector< double > time, intens;

    if ( spcfile_ && (spcfile_->number_of_spectra() > index) ) {

        if ( auto msdata = (*spcfile_)[index] ) {

            if ( spcfile_->getMS( *msdata, time, intens ) ) {
                size_t npts = time.size();

                auto ms = std::make_shared< adcontrols::MassSpectrum >();
                ms->resize( npts );
                ms->setAcquisitionMassRange( time[ 0 ], time[ npts - 1 ] );
                ms->setMassArray( time.data() );
                ms->setIntensityArray( intens.data());

                if ( auto spw = findChild< adplot::SpectrumWidget * >() ) {
                    // spw->setTitle( (boost::format( "%1%[%2%/%3%]" ) % fpath_ % (index_ + 1) % spcfile_->spchdr()->number_of_subfiles()).str() );
                    spw->setKeepZoomed( false );
                    spw->setData( ms, 0 );
                }

            }
        }
    }

#if 0
    if ( index < spcfile_->number_of_subfiles() )
        index_ = index;
    else
        index_ = 0;

    std::pair< double, double > range = std::make_pair( spcfile_->spchdr()->ffirst(), spcfile_->spchdr()->flast() );
    std::ostringstream o;
    if ( auto sub = spcfile_->subhdr( index_ ) ) {
        sub->dump_subhdr( o );

        const size_t npts = spcfile_->spchdr()->fnpts();
        std::shared_ptr< adcontrols::MassSpectrum > ms = std::make_shared< adcontrols::MassSpectrum >();
        ms->resize( npts );
        ms->setAcquisitionMassRange( range.first, range.second );

        for ( size_t i = 0; i < ms->size(); ++i ) {
            ms->setMass( i, i * double( range.second - range.first ) / ( npts - 1 ) + range.first );
            ms->setIntensity( i, (*sub)[i] );
        }
        if ( auto spw = findChild< adplot::SpectrumWidget * >() ) {
            spw->setTitle( ( boost::format( "%1%[%2%/%3%]" ) % fpath_ % (index_ + 1) % spcfile_->spchdr()->number_of_subfiles() ).str() );
            spw->setKeepZoomed( false );
            spw->setData( ms, 0 );
        }
        
        if ( QTextEdit * logw = findChild<QTextEdit *>() ) {
            logw->clear();
            logw->setText( o.str().c_str() );
        }
    }
#endif
}

void
MainWindow::initialSetup()
{
    boost::filesystem::path dir = user_preference::path( settings_.get() );
    if ( !boost::filesystem::exists( dir ) ) {
        if ( !boost::filesystem::create_directories( dir ) ) {
            QMessageBox::information( 0, "MainWindow"
                                      , QString( "Work directory '%1' can not be created" ).arg( dir.string().c_str() ) );
        }
    }
}

void
MainWindow::finalClose()
{
    // auto path = user_preference::path( impl_->settings_.get() ) / "default_method.xml"; 
    // path.replace_extension( ".xml" );
    // boost::filesystem::wofstream of( path );

    // if ( mpxcontrols::Mpx4Method::xml_archive( of, *impl_->method_ ) ) {
    //     qtwrapper::settings( *impl_->settings_ ).addRecentFiles( Constants::GRP_METHOD_FILES
    //                                                              , Constants::KEY_FILES
    //                                                              , path.string().c_str() );
    // }
}
