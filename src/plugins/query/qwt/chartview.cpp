/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
**************************************************************************/

#include "chartview.hpp"
#include "xyseriesdata.hpp"
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_picker.h>
#include <qwt_plot_zoomer.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_draw.h>
#include <boost/format.hpp>
#include <ratio>
#include <memory>

using namespace query::qwt;

class Zoomer: public QwtPlotZoomer
{
public:
    Zoomer( QWidget *canvas ) : QwtPlotZoomer( canvas )  {
        setTrackerMode( AlwaysOn );
    }

    QwtText trackerTextF( const QPointF &pos ) const {
        QColor bg( Qt::white );
        bg.setAlpha( 200 );
        
        QwtText text = QwtPlotZoomer::trackerTextF( pos );
        text.setBackgroundBrush( QBrush( bg ) );
        return text;
    }
};

ChartView::ChartView( QWidget * parent ) : QwtPlot( parent )
{
    // void setupPalette()
    {
        QPalette pal = palette();
        
        QLinearGradient gradient;
        gradient.setCoordinateMode( QGradient::StretchToDeviceMode );
        gradient.setColorAt( 0.0, QColor( 0, 49, 110 ) );
        gradient.setColorAt( 1.0, QColor( 0, 87, 174 ) );
        
        pal.setBrush( QPalette::Window, QBrush( gradient ) );

        // QPalette::WindowText is used for the curve color
        pal.setColor( QPalette::WindowText, Qt::green );

        setPalette( pal );
    }
    
    // setCanvasBackground( QColor( "#d0d0d0" ) );
    // curve_->setStyle( QwtPlotCurve::Lines );
    // curve_->setPen( canvas()->palette().color( QPalette::WindowText ) );
    // curve_->setRenderHint( QwtPlotItem::RenderAntialiased, true );
    // curve_->setLegendAttribute( QwtPlotCurve::LegendShowLine );
    // curve_->setYAxis( QwtPlot::yRight );
    // curve_->attach( this );

    QwtPlotGrid *grid = new QwtPlotGrid();
    grid->setPen( Qt::gray, 0.0, Qt::DotLine );
    grid->enableX( true );
    grid->enableXMin( true );
    grid->enableY( true );
    grid->enableYMin( false );
    grid->attach( this );
    
    //auto zoomer = new Zoomer( this );
    auto zoomer = new QwtPlotZoomer( QwtPlot::xBottom, QwtPlot::yLeft, this->canvas() );

    // Shift+LeftButton: zoom out to full size
    // Ctrl+LeftButton: zoom out by 1
    // in addition to this, double click for zoom out by 1 via override widgetMouseDoubleClickEvent    
    zoomer->setMousePattern( QwtEventPattern::MouseSelect2,  Qt::LeftButton, Qt::ShiftModifier );
    zoomer->setMousePattern( QwtEventPattern::MouseSelect3, Qt::LeftButton, Qt::ControlModifier );

    QwtPlotPanner *panner = new QwtPlotPanner( canvas() );
    panner->setAxisEnabled( QwtPlot::yRight, false );
    panner->setMouseButton( Qt::MidButton );

    const QFontMetrics fm( axisWidget( QwtPlot::yLeft )->font() );
    QwtScaleDraw *sd = axisScaleDraw( QwtPlot::yLeft );
    sd->setMinimumExtent( fm.width( "100.00" ) );

    const QColor c( Qt::darkBlue );
    zoomer->setRubberBandPen( c );
    zoomer->setTrackerPen( c );

    setAxisAutoScale( QwtPlot::yLeft, true );
    setAxisAutoScale( QwtPlot::xBottom, true );
}

ChartView::~ChartView()
{
}

void
ChartView::setData( QAbstractItemModel * model, const QString& title, int x, int y
                    , const QString& xLabel, const QString& yLabel, const QString& chartType )
{
    QRectF bRect;
    
    if ( model ) {
        
        if ( chartType == "Scatter" ) {

            auto curve = std::make_shared< QwtPlotCurve >();
            plots_.emplace_back( curve );

            auto series = new XYScatterData( model, x, y );
            curve->setSamples( series );
            curve->attach( this );
            bRect = series->boundingRect();

        } else if ( chartType == "Line" ) {

            auto curve = std::make_shared< QwtPlotCurve >();
            plots_.emplace_back( curve );

            curve->setStyle( QwtPlotCurve::Lines );
            curve->setPen( canvas()->palette().color( QPalette::WindowText ) );
            curve->setRenderHint( QwtPlotItem::RenderAntialiased, true );
            curve->setLegendAttribute( QwtPlotCurve::LegendShowLine );
            curve->setYAxis( QwtPlot::yRight );

            auto series = new XYScatterData( model, x, y );
            curve->setSamples( series );
            curve->attach( this );
            bRect = series->boundingRect();
            
        } else if ( chartType == "Histogram" ) {

            auto curve = std::make_shared< QwtPlotCurve >();
            plots_.emplace_back( curve );            

            curve->setStyle( QwtPlotCurve::Lines );
            curve->setPen( canvas()->palette().color( QPalette::WindowText ) );
            curve->setRenderHint( QwtPlotItem::RenderAntialiased, true );
            curve->setLegendAttribute( QwtPlotCurve::LegendShowLine );
            curve->setYAxis( QwtPlot::yRight );

            auto series = new XYHistogramData( model, x, y );
            curve->setSamples( series );
            curve->attach( this );
            bRect = series->boundingRect();
        }

        setAxisScale( QwtPlot::yLeft,   bRect.bottom(), bRect.top() );
        setAxisScale( QwtPlot::xBottom, bRect.left(), bRect.right() );

        if ( auto zoomer = findChild< QwtPlotZoomer * >() )
            zoomer->setZoomBase();

        // if ( auto axisX = chart->axisX() )
        //     axisX->setTitleText( xLabel );
        
        // if ( auto axisY = chart->axisY() )
        //     axisY->setTitleText( yLabel );            
    }
    
}

void
ChartView::clear()
{
    plots_.clear();
}

