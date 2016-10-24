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
#include <adplot/zoomer.hpp>
#include <qwt_picker_machine.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_picker.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_renderer.h>
#include <qwt_scale_engine.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_draw.h>
#include <qwt_series_data.h>
#include <qwt_symbol.h>
#include <QApplication>
#include <QBuffer>
#include <QClipboard>
#include <QFileDialog>
#include <QMenu>
#include <QMimeData>
#include <QMouseEvent>
#include <QSvgGenerator>
#include <boost/format.hpp>
#include <ratio>
#include <memory>

namespace query {
    namespace qwt {

        static Qt::GlobalColor color_table[] = {
            Qt::blue            // 0
            , Qt::red           // 1
            , Qt::darkGreen     // 2
            , Qt::darkCyan      // 3
            , Qt::darkMagenta   // 4
            , Qt::darkYellow    // 5
            , Qt::darkBlue      // 6
            , Qt::darkRed       // 7            
            , Qt::green         // 8
            , Qt::cyan          // 9
            , Qt::magenta       // 10
            , Qt::yellow        // 11
            , Qt::darkGray      // 12
            , Qt::black         // 13
            , Qt::lightGray     // 14
            , Qt::white         // 15
            , Qt::gray          // 16
            , Qt::transparent   // 17
        };

    }
}

using namespace query::qwt;

ChartView::ChartView( QWidget * parent ) : QwtPlot( parent )
{
    // void setupPalette()
     {
         QPalette pal = palette();
         QLinearGradient gradient;
         gradient.setCoordinateMode( QGradient::StretchToDeviceMode );
         // gradient.setColorAt( 0.0, QColor( 0xcf, 0xcf, 0xc4 ) ); // pastel gray
         // gradient.setColorAt( 1.0, QColor( 0xae, 0xc6, 0xcf ) ); // pastel blue
         gradient.setColorAt( 0.0, QColor( 0xc1, 0xff, 0xc1 ) ); // darkseagreen
         gradient.setColorAt( 1.0, QColor( 0xb4, 0xee, 0xb4 ) ); // darkseagreen 2
         
         pal.setBrush( QPalette::Window, QBrush( gradient ) );
         
         // QPalette::WindowText is used for the curve color
         // pal.setColor( QPalette::WindowText, Qt::green );

         setPalette( pal );
     }

    this->enableAxis( QwtPlot::yLeft, true );
    this->enableAxis( QwtPlot::xBottom, true );
    
    QwtPlotGrid *grid = new QwtPlotGrid();
    grid->setPen( Qt::gray, 0.0, Qt::DotLine );
    grid->enableX( true );
    grid->enableXMin( true );
    grid->enableY( true );
    grid->enableYMin( false );
    grid->attach( this );

    this->axisScaleEngine( QwtPlot::yLeft )->setAttribute( QwtScaleEngine::Floating, true );
    this->axisScaleEngine( QwtPlot::xBottom )->setAttribute( QwtScaleEngine::Floating, true );
    
    auto zoomer = new adplot::Zoomer( QwtPlot::xBottom, QwtPlot::yLeft, this->canvas() );

    // Shift+LeftButton: zoom out to full size
    // Ctrl+LeftButton: zoom out by 1
    zoomer->setMousePattern( QwtEventPattern::MouseSelect2, Qt::LeftButton, Qt::ShiftModifier );
    zoomer->setMousePattern( QwtEventPattern::MouseSelect3, Qt::LeftButton, Qt::ControlModifier );

    const QColor c( Qt::darkBlue );
    zoomer->setRubberBandPen( c );
    zoomer->setTrackerPen( c );
    zoomer->autoYScale( false );

    if ( auto panner = new QwtPlotPanner( canvas() ) ) {
        panner->setAxisEnabled( QwtPlot::yRight, false );
        panner->setMouseButton( Qt::MidButton );
    }

    if ( auto picker = new QwtPlotPicker( canvas() ) ) {
        picker->setMousePattern( QwtEventPattern::MouseSelect1,  Qt::RightButton );
        picker->setStateMachine( new QwtPickerClickPointMachine() );
        connect( picker, static_cast< void(QwtPlotPicker::*)(const QPointF&) >(&QwtPlotPicker::selected), this, &ChartView::selected );
        picker->setEnabled( true );
    }
}

ChartView::~ChartView()
{
}

void
ChartView::setData( QAbstractItemModel * model, const QString& title, int x, int y
                    , const QString& xLabel, const QString& yLabel, const QString& chartType )
{
    if ( model ) {

        size_t size = plots_.size();
        auto color = color_table[ size % ( sizeof( color_table ) / sizeof( color_table[ 0 ] ) ) ];
        
        if ( chartType == "Scatter" ) {

            auto curve = std::make_shared< QwtPlotCurve >();
            curve->setStyle( QwtPlotCurve::NoCurve );
            curve->setSymbol( new QwtSymbol( QwtSymbol::XCross, Qt::NoBrush, QPen( color ), QSize( 5, 5 ) ) );
            plots_.emplace_back( curve );

            curve->setSamples( new XYSeriesData( model, x, y ) );
            curve->attach( this );

        } else if ( chartType == "Line" ) {

            auto curve = std::make_shared< QwtPlotCurve >();
            plots_.emplace_back( curve );

            curve->setStyle( QwtPlotCurve::Lines );
            curve->setPen( QPen( color ) );
            curve->setRenderHint( QwtPlotItem::RenderAntialiased, true );
            curve->setLegendAttribute( QwtPlotCurve::LegendShowLine );
            curve->setYAxis( QwtPlot::yRight );

            curve->setSamples( new XYSeriesData( model, x, y ) );
            curve->attach( this );
            
        } else if ( chartType == "Histogram" ) {

            auto curve = std::make_shared< QwtPlotCurve >();
            plots_.emplace_back( curve );            

            curve->setStyle( QwtPlotCurve::Sticks );
            curve->setPen( QPen( color ) );            
            curve->setLegendAttribute( QwtPlotCurve::LegendShowLine );
            curve->setYAxis( QwtPlot::yRight );

            curve->setSamples( new XYHistogramData( model, x, y ) );
            curve->attach( this );
        }

        if ( plots_.size() != size ) {

            QRectF bRect;

            for ( auto& plot: plots_ )
                bRect |= plot->boundingRect();
            
            this->setAxisScale( QwtPlot::yLeft, std::min( bRect.top(), bRect.bottom() ), std::max( bRect.top(), bRect.bottom() ) );
            this->setAxisScale( QwtPlot::xBottom, bRect.left(), bRect.right() );

            if ( auto zoomer = findChild< QwtPlotZoomer * >() )
                zoomer->setZoomBase();

            setAxisTitle( QwtPlot::xBottom, QwtText( xLabel, QwtText::RichText ) );
            setAxisTitle( QwtPlot::yLeft, QwtText( yLabel, QwtText::RichText ) );
        }
    }
    
}

void
ChartView::clear()
{
    plots_.clear();
}


void
ChartView::copyToClipboard()
{
    auto sz = this->size();

    QImage img( sz, QImage::Format_ARGB32 );
    img.fill( qRgba( 0, 0, 0, 0 ) );
    
    QRectF rc( 0, 0, sz.width(), sz.height() );
    
    QwtPlotRenderer renderer;
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasBackground, false );
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasFrame, true );
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardBackground, true );
    
    QPainter painter;

    painter.begin(&img);
    renderer.render( this, &painter, this->rect() );
    painter.end();

    if ( auto clipboard = QApplication::clipboard() )
        clipboard->setImage( img );
}

void
ChartView::saveImage( bool clipboard )
{
    QSvgGenerator generator;
    QByteArray svg;
    QBuffer buffer( &svg );
    
    generator.setTitle( "QtPlatz Generated SVG" );
    generator.setDescription( "Copyright (C) 2013-2017 MS-Cheminformataics, All rights reserved" );
    auto sz = this->size();
    QRectF rc( 0, 0, sz.width(), sz.height() );
    generator.setViewBox( rc );

    if ( clipboard ) {

        generator.setOutputDevice( &buffer );        

    } else {
        auto name = QFileDialog::getSaveFileName( this, tr( "Save SVG File" )
                                                  , "chart.svg"
                                                  , tr( "SVG (*.svg)" ) );
        if ( ! name.isEmpty() )
            generator.setFileName( name );
    }

    QwtPlotRenderer renderer;

    renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasBackground, true );
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasFrame, true );
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardBackground, true );

    QPainter painter;
    painter.begin( &generator );
    renderer.render( this, &painter, rc );
    painter.end();

    if ( clipboard ) {
        QMimeData * mime = new QMimeData();
        mime->setData( "image/svg+xml", svg );
        QApplication::clipboard()->setMimeData( mime, QClipboard::Clipboard );
    }
}

void
ChartView::selected( const QPointF& pos )
{
    QMenu menu;

    int idx(0);
    menu.addAction( tr( "Unzoom" ) )->setData( idx++ );                 // 0
    menu.addAction( tr( "Copy x-coordinate" ) )->setData( idx++ );      // 1
    menu.addAction( tr( "Copy x,y-coordinate" ) )->setData( idx++ );    // 2
    menu.addAction( tr( "Copy image" ) )->setData( idx++ );             // 3
    menu.addAction( tr( "Copy SVG" ) )->setData( idx++ );               // 4
    menu.addAction( tr( "Save as SVG File..." ) )->setData( idx++ );    // 5

    if ( auto selected = menu.exec( QCursor::pos() ) ) {

        switch ( selected->data().toInt() ) {
        case 0:
            if ( auto zoomer = findChild< QwtPlotZoomer * >() )
                zoomer->setZoomStack( zoomer->zoomStack(), 0 );
            break;
        case 1:
            QApplication::clipboard()->setText( QString::number( pos.x(), 'g', 14 ) );
            break;
        case 2:
            QApplication::clipboard()->setText( QString( "%1, %2" ).arg( QString::number( pos.x(), 'g', 14 )
                                                                         , QString::number( pos.y(), 'g', 14 ) ) );
            break;
        case 3:
            copyToClipboard();
            break;
        case 4:
            saveImage( true );
            break;
        case 5:
            saveImage( false );
            break;
        }
    }

}
