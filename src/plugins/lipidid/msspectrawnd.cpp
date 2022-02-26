/**************************************************************************
** Copyright (C) 2022-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2022-2022 MS-Cheminformatics LLC
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

#include "msspectrawnd.hpp"
#include "document.hpp"
#include "mainwindow.hpp"
#include <adcontrols/datafile.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msqpeaks.hpp>
#include <adlog/logger.hpp>
#include <adplot/chromatogramwidget.hpp>
#include <adplot/panner.hpp>
#include <adplot/peakmarker.hpp>
#include <adplot/spectrogramwidget.hpp>
#include <adplot/spectrumwidget.hpp>
#include <adplot/zoomer.hpp>
#include <adplugin/lifecycle.hpp>
#include <adplugin/plugin.hpp>
#include <adplugin/plugin_ptr.hpp>
#include <adportable/array_wrapper.hpp> // std::span
#include <adportable/configuration.hpp>
#include <adportable/debug.hpp>
#include <adportable/float.hpp>
#include <adportfolio/folder.hpp>
#include <adportfolio/folium.hpp>
#include <adportfolio/portfolio.hpp>
#include <adprocessor/dataprocessor.hpp>
#include <adutils/processeddata.hpp>
#include <adwidgets/mspeaktree.hpp>
#include <adwidgets/msquantable.hpp>
#include <boost/variant.hpp>
#include <boost/any.hpp>
#include <boost/exception/all.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <adportable/unique_ptr.hpp>
#include <qtwrapper/waitcursor.hpp>
#include <qtwrapper/settings.hpp>
#include <qwt_scale_widget.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_renderer.h>
#include <qwt_plot_marker.h>
#include <qwt_symbol.h>
#include <algorithm>
#include <cmath>

#include <coreplugin/minisplitter.h>
#include <QBoxLayout>
#include <QFileDialog>
#include <QMenu>
#include <QPrinter>
#include <functional>

namespace lipidid {

    class MSSpectraWnd::impl {
    public:
        impl( MSSpectraWnd * p ) : pThis_( p )
                                 , isTimeAxis_( false )
                                 , dirty_( false )
                                 , selProcessed_( false ) {

            for ( size_t i = 0; i < plots_.size(); ++i ) {
                plots_[ i ] = std::make_unique< adplot::SpectrumWidget >();
                plots_[ i ]->axisWidget( QwtPlot::yLeft )->scaleDraw()->setMinimumExtent( 80 );
                plots_[ i ]->axisWidget( QwtPlot::yRight )->scaleDraw()->setMinimumExtent( 60 );
                markers_[ i ] = std::make_unique< adplot::PeakMarker >();
                QObject::connect( plots_[ i ]->zoomer(), &adplot::Zoomer::zoomed
                                  , [=]( const QRectF& rc ){
                    emit document::instance()->onZoomed( i, rc );
                } );
            }
        }

        ~impl() {
        }

        MSSpectraWnd * pThis_;
        std::array< std::unique_ptr< adplot::SpectrumWidget >, 2 > plots_;
        std::array< std::unique_ptr< adplot::PeakMarker >, 2 > markers_;
        bool isTimeAxis_;
        bool dirty_;
        bool selProcessed_;
        std::string selectedFormula_;
    };

}

using lipidid::MSSpectraWnd;

MSSpectraWnd::~MSSpectraWnd()
{
    delete impl_;
}

MSSpectraWnd::MSSpectraWnd( QWidget *parent ) : QWidget(parent)
                                              , impl_( new impl( this ) )
{
    init();
    setContextMenuPolicy( Qt::CustomContextMenu );
}

void
MSSpectraWnd::init()
{
    Core::MiniSplitter * splitter = new Core::MiniSplitter;
    if ( splitter ) {

        for ( size_t i = 0; i < impl_->plots_.size(); ++i ) {

            auto& plot = impl_->plots_[i];
            auto& marker = impl_->markers_[i];

            connect( plot.get()
                     , static_cast< void(adplot::SpectrumWidget::*)(const QRectF&)>(&adplot::SpectrumWidget::onSelected)
                     , [&plot,this]( const QRectF& rc ) { handleSelected( rc, plot.get() ); } );

            // plot->enableAxis( QwtPlot::yRight );
            plot->setMinimumHeight( 80 );
            marker->attach( plot.get() );
            marker->visible( true );
            marker->setYAxis( QwtPlot::yRight );

            if ( i )
                impl_->plots_[ 0 ]->link( plot.get() );

            splitter->addWidget( plot.get() );

        }

        // splitter->addWidget( impl_->table_.get() );
        splitter->setOrientation( Qt::Vertical );
    }

    QBoxLayout * toolBarAddingLayout = new QVBoxLayout( this );
    toolBarAddingLayout->setMargin(0);
    toolBarAddingLayout->setSpacing(0);
    toolBarAddingLayout->addWidget( splitter );
}


void
MSSpectraWnd::handleSessionAdded( adprocessor::dataprocessor * processor )
{
}

void
MSSpectraWnd::handleSelectionChanged( adprocessor::dataprocessor * processor, portfolio::Folium& folium )
{
    if ( ! portfolio::is_type< adcontrols::MassSpectrumPtr >( folium ) )
        return;
}

void
MSSpectraWnd::handleSelected( const QRectF& rc, adplot::SpectrumWidget * plot )
{
    auto d = std::abs( plot->transform( QwtPlot::xBottom, rc.left() ) - plot->transform( QwtPlot::xBottom, rc.right() ) );
    if ( d <= 2 ) {
		QMenu menu;
        typedef std::pair < QAction *, std::function<void()> > action_type;
        std::vector < action_type > actions;

        actions.emplace_back( menu.addAction( tr("Copy image to clipboard") ), [=] () { adplot::plot::copyToClipboard( plot ); } );
        actions.emplace_back( menu.addAction( tr( "Save SVG File" ) ) , [=] () {
            auto name = MainWindow::makeSaveSvgFilename();
            if ( ! name.isEmpty() ) {
                adplot::plot::copyImageToFile( plot, name, "svg" );
                qtwrapper::settings( *document::instance()->settings() ).addRecentFiles( "SVG", "Files", name );
            }
        });

        QAction * selected = menu.exec( QCursor::pos() );
        if ( selected ) {
            auto it = std::find_if( actions.begin(), actions.end(), [selected] ( const action_type& a ){ return a.first == selected; } );
            if ( it != actions.end() )
                (it->second)();
        }
    }
}

void
MSSpectraWnd::redraw()
{
}

void
MSSpectraWnd::handleApplyMethod( const adcontrols::ProcessMethod& )
{
}

void
MSSpectraWnd::handleCheckStateChanged( adprocessor::dataprocessor *, portfolio::Folium&, bool )
{
}

void
MSSpectraWnd::handleCurrentChanged( const QString& guid, int idx, int fcn )
{
    // deprecated -- used to work with quan table
}

void
MSSpectraWnd::handleDataChanged( const portfolio::Folium& folium )
{
    impl_->selectedFormula_.clear();

    using portfolio::is_any_shared_of;
    if ( is_any_shared_of< adcontrols::MassSpectrum, const adcontrols::MassSpectrum >( folium ) ) {
        using portfolio::get_shared_of;
        if ( auto ptr = get_shared_of< const adcontrols::MassSpectrum, adcontrols::MassSpectrum >()( folium.data() ) ) {
            if ( ptr->isCentroid() ) {
                impl_->plots_[ 0 ]->setData( ptr, 0, QwtPlot::yLeft );
            }
        }
    }
}

void
MSSpectraWnd::handleIdCompleted()
{
    impl_->selectedFormula_.clear();

    if ( auto refms = document::instance()->reference_mass_spectrum() ) {
        impl_->plots_[ 1 ]->setData( refms, 0, QwtPlot::yLeft );
    }
}

void
MSSpectraWnd::onPageSelected()
{
    if ( impl_->dirty_ ) {
        redraw();
    }
}

void
MSSpectraWnd::handleAxisChanged( adcontrols::hor_axis )
{
}

void
MSSpectraWnd::handleProcessed( adprocessor::dataprocessor * processor, portfolio::Folium& folium )
{
    handleSelectionChanged( processor, folium );
}

void
MSSpectraWnd::onDataChanged( const QString& foliumGuid, const QString& attGuid, int idx, int fcn )
{
    // data changed on MSPeakTable via MSProcessingWnd
}

void
MSSpectraWnd::handleDataChanged( const QString& dataGuid, int idx, int fcn, int column, const QVariant& data )
{
}

///////////////////////////
void
MSSpectraWnd::handlePrintCurrentView( const QString& pdfname )
{
}

void
MSSpectraWnd::handleFormulaSelection( const QString& formula, double abundance )
{
    if ( auto refms = document::instance()->reference_mass_spectrum() ) {
        impl_->plots_[ 1 ]->setAlpha( 0, 0x60 );
        if ( auto overlay = document::instance()->overlay_mass_spectrum() ) {
            impl_->plots_[ 1 ]->setData( overlay, 1, QwtPlot::yLeft );

            auto [left, right] = std::make_pair( overlay->massArray().front(), overlay->massArray().back() );

            auto rc = impl_->plots_[ 1 ]->zoomRect();
            auto rc1( rc );
            if ( right < rc.left() || rc.right() < left ) {
                if ( rc.right() < left ) { // all peaks are right-side on view mass range
                    rc.moveRight( right + ( rc.width() / 10 ) );
                } else if ( right < rc.left() ) { // all peaks are left-side on view mass range
                    rc.moveLeft( left - ( rc.width() / 10 ) );
                }
                ADDEBUG() << std::pair( rc1.left(), rc1.right() ) << " --> " << std::pair( rc.left(), rc.right() );
                QSignalBlocker block( document::instance() ); // block document::onZoomed, which is initiated from MSSpectraWnd::impl
                impl_->plots_[ 1 ]->zoomer()->zoom( rc );
            }
        }
    }
}
