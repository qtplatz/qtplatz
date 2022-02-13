/**************************************************************************
** Copyright (C) 2010-2021 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2021 MS-Cheminformatics LLC
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
#include <adprocessor/dataprocessor.hpp>
#include "document.hpp"
#include "mainwindow.hpp"
#include <adcontrols/datafile.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msqpeaks.hpp>
#include <adlog/logger.hpp>
#include <adplot/chromatogramwidget.hpp>
#include <adplot/peakmarker.hpp>
#include <adplot/spectrogramwidget.hpp>
#include <adplot/spectrumwidget.hpp>
#include <adplugin/lifecycle.hpp>
#include <adplugin/plugin.hpp>
#include <adplugin/plugin_ptr.hpp>
#include <adportable/array_wrapper.hpp> // std::span
#include <adportable/configuration.hpp>
#include <adportable/float.hpp>
#include <adportable/debug.hpp>
#include <adportfolio/folder.hpp>
#include <adportfolio/folium.hpp>
#include <adportfolio/portfolio.hpp>
#include <adutils/processeddata.hpp>
#include <adwidgets/mspeaktree.hpp>
#include <adwidgets/msquantable.hpp>
#include <boost/variant.hpp>
#include <boost/any.hpp>
#include <boost/exception/all.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <adportable/unique_ptr.hpp>
#include <qtwrapper/waitcursor.hpp>
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
    ADDEBUG() << "## " << __FUNCTION__ << folium.fullpath();

    // std::shared_ptr< const adcontrols::MassSpectrum > ms;
    // if ( portfolio::is_type< adcontrols::MassSpectrumPtr >( folium ) ) {
    //     ms = portfolio::get< std::shared_ptr< const adcontrols::MassSpectrum > >( folium );
    // } else if ( portfolio::is_type< adcontrols::MassSpectrumPtr >( folium ) ) {
    //     ms = portfolio::get< std::shared_ptr< adcontrols::MassSpectrum > >( folium );
    // }
    // if ( ms ) {
    //     ADDEBUG() << "handleDataChanged: " << ms->size() << ", " << ms->isCentroid();
    //     impl_->plots_[ 0 ]->setData( ms, 0 );
    // }
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
