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
#include "../spcfile/spcfile.hpp"
#include "../spcfile/spchdr.hpp"
#include "../spcfile/subhdr.hpp"
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adwplot/spectrumwidget.hpp>
#include <adportable/profile.hpp>
#include <QAction>
#include <QBoxLayout>
#include <QDockWidget>
#include <QFileDialog>
#include <QLabel>
#include <QMenuBar>
#include <QMenu>
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

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
                                        , index_( 0 )
{
	this->resize( 800, 600 );
    QWidget * centralWidget = new QWidget( this );
    centralWidget->setObjectName( "centralWidget" );
    this->setCentralWidget( centralWidget );
    this->setStatusBar( new QStatusBar( this ) );
    this->setMenuBar( new QMenuBar( this ) );

	if ( QMenu * fileMenu = this->menuBar()->addMenu( tr("&File") ) ) {
		fileMenu->addAction( tr("Open Grams .spc file..."), this, SLOT( actFileOpen() ) );
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
    toolBarLayout->addWidget( new adwplot::SpectrumWidget );
    toolBarLayout->addWidget( toolBar );

    this->addDockWidget( Qt::BottomDockWidgetArea, addDockForWidget( new QTextEdit ) );
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
        if ( spcfile_ = std::make_shared< galactic::spcfile >( in, fsize ) ) {
            dumpspc( *spcfile_, o );
        }
        draw( 0 );
        return true;
    }
    return false;
}

void
MainWindow::dumpspc( const galactic::spcfile& spc, std::ostream& o )
{
    if ( auto hdr = spc.spchdr() )
        hdr->dump_spchdr( o );
    if ( auto sub = spc.subhdr() )
        sub->dump_subhdr( o );
}

void
MainWindow::actFileOpen()
{
    boost::filesystem::path datapath( adportable::profile::user_data_dir<char>() );
    
    QString name =
        QFileDialog::getOpenFileName( this
                                      , tr("Open Grams .spc file")
                                      , datapath.string().c_str()
                                      , tr("Galactic SPC Files(*.spc)") );
    if ( !name.isEmpty() ) {
        if ( adwplot::SpectrumWidget * w = findChild< adwplot::SpectrumWidget * >() ) {
            fpath_ = name.toStdString();
            w->setTitle( fpath_ );

            if ( Open( name.toStdString() ) ) {
                std::ostringstream o;
                o << spcfile_->source_instrument_description() << "\t" << spcfile_->date();
				w->setFooter( o.str() );
                if ( const galactic::spchdr * h = spcfile_->spchdr() ) {
                    w->setAxisTitle( QwtPlot::xBottom, QwtText( spcfile_->axis_x_label() ) );
                    w->setAxisTitle( QwtPlot::yLeft, QwtText( spcfile_->axis_y_label() ) );
                }
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
MainWindow::draw( size_t index )
{
    if ( index < spcfile_->number_of_subfiles() )
        index_ = index;
    else
        index_ = 0;

    std::ostringstream o;
    if ( auto sub = spcfile_->subhdr( index_ ) )
        sub->dump_subhdr( o );

    auto * data = reinterpret_cast< const int32_t * >( spcfile_->subhdr( index_ )->data() );
    int fexp = spcfile_->spchdr()->fexp();
    
    std::shared_ptr< adcontrols::MassSpectrum > ms = std::make_shared< adcontrols::MassSpectrum >();
    
    ms->resize( spcfile_->spchdr()->fnpts() );
    std::pair< double, double > range = std::make_pair( spcfile_->spchdr()->ffirst(), spcfile_->spchdr()->flast() );
    ms->setAcquisitionMassRange( range.first, range.second );
    for ( size_t i = 0; i < ms->size(); ++i ) {
        ms->setMass( i, i * ( range.second - range.first ) / (ms->size() - 1) + range.first );
        ms->setIntensity( i, double( int64_t(data[i]) << fexp ) / double(0xffffffffL) );
    }
    if ( auto spw = findChild< adwplot::SpectrumWidget * >() ) {
        spw->setTitle( ( boost::format( "%1%[%2%/%3%]" ) % fpath_ % (index_ + 1) % spcfile_->spchdr()->number_of_subfiles() ).str() );
        spw->setKeepZoomed( false );
        spw->setData( ms, 0 );
    }
    
    if ( QTextEdit * logw = findChild<QTextEdit *>() ) {
        logw->clear();
        logw->setText( o.str().c_str() );
    }
}

