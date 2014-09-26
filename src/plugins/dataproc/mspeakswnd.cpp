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

#include "mspeakswnd.hpp"
#include "mainwindow.hpp"
#include <adcontrols/mspeaks.hpp>
#include <adcontrols/mspeak.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adplot/plot.hpp>
#include <adplot/zoomer.hpp>
#include <adportable/polfit.hpp>
#include <adportable/float.hpp>
#include <qtwrapper/font.hpp>
#include <qwt_scale_widget.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_legenditem.h>
#include <qwt_plot_renderer.h>
#include <qwt_symbol.h>
#include <qwt_scale_engine.h>
#include <qwt_legend.h>

#include <coreplugin/minisplitter.h>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QPrinter>
#include <boost/format.hpp>
#include <sstream>
#include <tuple>

namespace dataproc {
    namespace mspeakswnd {
        
        struct draw_crosshair_marker {
            std::vector< std::shared_ptr< QwtPlotMarker > >& markers_;
            draw_crosshair_marker( std::vector< std::shared_ptr< QwtPlotMarker > >& markers ) : markers_( markers ) {}
            void operator ()(double x, double y, QwtPlot& plot) {
                std::shared_ptr< QwtPlotMarker > marker = std::make_shared< QwtPlotMarker >();
                markers_.push_back( marker );
                marker->setValue( x, y );
                // marker->setLineStyle( QwtPlotMarker::Cross );
                marker->setLinePen( Qt::darkGreen, 0.0, Qt::DashDotLine );
                marker->setLabel( QwtText( (boost::format("%.7g<i>(ns)</i>") % ( y * 1000 )).str().c_str(), QwtText::RichText ) );
                marker->setLabelAlignment( Qt::AlignRight | Qt::AlignBottom );
                marker->attach( &plot );
            }
        };

        struct draw_regression {
            std::vector< std::shared_ptr< QwtPlotCurve > >& curves_;
            draw_regression( std::vector< std::shared_ptr< QwtPlotCurve > >& curves ) : curves_( curves ) {}

            void operator ()( const QVector< QPointF>& data, const std::string& label, QwtPlot& plot ) {

                std::shared_ptr< QwtPlotCurve > curve = std::make_shared< QwtPlotCurve >( label.c_str() );
                curves_.push_back( curve );

                curve->setPen( Qt::darkGreen );
                curve->setLegendAttribute( QwtPlotCurve::LegendShowLine );
                curve->setYAxis( QwtPlot::yLeft );
                curve->setSamples( data );
                curve->attach( &plot );
            }
        };

        struct draw_stics {
            std::vector< std::shared_ptr< QwtPlotCurve > >& curves_;
            std::vector< std::shared_ptr< QwtPlotMarker > >& markers_;

            draw_stics( std::vector< std::shared_ptr< QwtPlotCurve > >& curves
                        , std::vector< std::shared_ptr< QwtPlotMarker > >& markers) : curves_( curves ), markers_( markers ) {}
            
            void operator ()( const QVector< QPointF>& data, QwtPlot& plot ) {

                const QPointF& dmax = *std::max_element( data.begin(), data.end(), []( const QPointF& a, const QPointF& b ){
                        return std::abs(a.y()) < std::abs(b.y()); } );
                double dfs = std::abs(dmax.y()) * 1.2;
                plot.setAxisScale( QwtPlot::yRight, -dfs, dfs );
                
                std::vector< std::tuple< int, double, double > > bars;
                for ( auto& datum: data ) {
                    std::shared_ptr< QwtPlotMarker > marker = std::make_shared< QwtPlotMarker >();
                    markers_.push_back( marker );
                    marker->setSymbol( new QwtSymbol( QwtSymbol::Style( QwtSymbol::Cross ), Qt::NoBrush, QPen( Qt::red ), QSize(5, 5) ) );
                    marker->setValue( datum.x(), datum.y() );
                    marker->setYAxis( QwtPlot::yRight );
                    marker->attach( &plot );
                    auto it = std::find_if( bars.begin(), bars.end(), [=]( const std::tuple<int, double, double>& d ){
                            return adportable::compare<double>::essentiallyEqual( std::get<1>(d), datum.x() );
                        });
                    if ( it == bars.end() )
                        bars.push_back( std::make_tuple( 1, datum.x(), datum.y() ) );
                    else {
                        std::get<0>( *it )++;
                        std::get<2>( *it ) += datum.y();
                    }
                }
                QVector< QPointF > bar;
                for ( auto& t: bars ) {
                    if ( std::get<0>(t) > 0 )
                        bar.push_back( QPointF( std::get<1>(t), std::get<2>(t) / std::get<0>(t) ) );
                }

                std::shared_ptr< QwtPlotCurve > curve = std::make_shared< QwtPlotCurve >( QwtText("&delta;(ns)", QwtText::RichText) );
                curves_.push_back( curve );
                curve->setPen( Qt::red );
                curve->setYAxis( QwtPlot::yRight );
                curve->setSamples( bar );
				curve->setStyle( QwtPlotCurve::Sticks );
                curve->attach( &plot );
                
                // horizontal center line for deviation plot
                std::shared_ptr< QwtPlotMarker > marker = std::make_shared< QwtPlotMarker >();
                markers_.push_back( marker );
                marker->setYAxis( QwtPlot::yRight );
                marker->setValue( 0.0, 0.0 );
                marker->setLineStyle( QwtPlotMarker::HLine );
                marker->setLinePen( Qt::gray, 0.0, Qt::DashDotLine );
                marker->attach( &plot );
            }
		};
    }
}

using namespace dataproc;

MSPeaksWnd::MSPeaksWnd(QWidget *parent) : QWidget(parent)
{
    plots_.push_back( std::make_shared< adplot::plot >() );
    plots_.push_back( std::make_shared< adplot::plot >() );
    
    plotMarkers_.resize( plots_.size() );
    plotCurves_.resize( plots_.size() );

    static struct {
        const char * xBottom;
        const char * yLeft;
    } axis_titles [] = {
        { "&radic;<span style=\"text-decoration: overline\">&nbsp;<i>m/z</i></span>"
          , "time (&mu;s)"
        }
        , { "flight length (m)"
          , "time (&mu;s)"
        }
    };
    assert( sizeof( axis_titles ) / sizeof( axis_titles[0] ) == plots_.size() );

    QFont font;
	qtwrapper::font::setFont( font, qtwrapper::fontSizeSmall, qtwrapper::fontAxisLabel );
    font.setFamily( "Consolas" );
    font.setBold( false );
	font.setPointSize( 8 );

    int n = 0;
    for ( auto& plot: plots_ ) {

        plot->setMinimumHeight( 40 );
        plot->setMinimumWidth( 40 );
        plot->enableAxis( QwtPlot::yRight );

        plot->setAxisFont( QwtPlot::xBottom, font );
        plot->setAxisFont( QwtPlot::yLeft, font );
        plot->setAxisFont( QwtPlot::yRight, font );

        plot->setAxisTitle( QwtPlot::yLeft, QwtText( axis_titles[ n ].yLeft, QwtText::RichText ) );
        plot->setAxisTitle( QwtPlot::xBottom, QwtText( axis_titles[ n ].xBottom, QwtText::RichText ) );
        plot->setAxisTitle( QwtPlot::yRight, QwtText( "&delta;(ns)", QwtText::RichText ) );

        plot->axisAutoScale( QwtPlot::xBottom );
        plot->axisAutoScale( QwtPlot::yLeft );
        ++n;

        QwtPlotGrid * grid = new QwtPlotGrid;
        grid->setMajorPen( Qt::gray, 0, Qt::DotLine );
        grid->attach( plot.get() );

        QwtPlotLegendItem * legendItem = new QwtPlotLegendItem;
        legendItem->attach( plot.get() );
    };

    init();
}

void
MSPeaksWnd::init()
{
    if ( Core::MiniSplitter * splitter = new Core::MiniSplitter ) {

        for ( auto& plot: plots_ )
            splitter->addWidget( plot.get() );

        splitter->setOrientation( Qt::Horizontal );

        QBoxLayout * layout = new QVBoxLayout( this );
        layout->setMargin( 0 );
        layout->setSpacing( 0 );
        layout->addWidget( splitter );
    }
}

void
MSPeaksWnd::handleSetData( int mode, const adcontrols::MSPeaks& peaks )
{
    adplot::plot& plot = *plots_[ 0 ];
    std::vector< std::shared_ptr< QwtPlotMarker > >& markers = plotMarkers_[ 0 ];
    std::vector< std::shared_ptr< QwtPlotCurve > >& curves = plotCurves_[ 0 ];

    markers.clear();
    curves.clear();
    
    // title
    plot.setTitle( ( boost::format( "#lap: %d" ) % mode ).str() );

    // SQRT(m) - time plot
    // footer (equation)
    std::ostringstream o;
    o << "&radic;<span style=\"text-decoration: overline\">&nbsp;<i>m/z</i></span> = ";
    const std::vector<double> coeffs = peaks.coeffs();
    o << boost::format( "%.7e + " ) % coeffs[0];
    o << boost::format( "%.7e&sdot;t" ) % coeffs[1];
    for ( size_t i = 2; i < coeffs.size(); ++i )
        o << boost::format( " + %.7e&sdot;t<sup>%d</sup>") % coeffs[i] % i;
    plot.setFooter( o.str() );

    // markers
    for ( size_t i = 0; i < peaks.x().size(); ++i ) {
        std::shared_ptr< QwtPlotMarker > marker = std::make_shared< QwtPlotMarker >( peaks[ i ].formula().c_str() );
        markers.push_back( marker );
        marker->setLabel( QwtText( adcontrols::ChemicalFormula::formatFormula( peaks[ i ].formula() ).c_str() ) );
        marker->setLabelAlignment( Qt::AlignRight | Qt::AlignBottom );
        marker->setValue( peaks.y()[ i ], peaks.x()[ i ] );
		marker->setSymbol( new QwtSymbol( QwtSymbol::Style( QwtSymbol::XCross ), Qt::NoBrush, QPen( Qt::darkMagenta ), QSize(5, 5) ) );
        marker->attach( &plot );
    }

    // deviation
    QVector< QPointF > deviation;
    for ( size_t i = 0; i < peaks.x().size(); ++i ) {
        double sqrtM = peaks.y()[i];
        double t = ( sqrtM - peaks.coeffs()[ 0 ] ) / peaks.coeffs()[ 1 ];
        deviation.push_back( QPointF( sqrtM, (peaks.x()[i] - t) * 1000 ) );
    }
    mspeakswnd::draw_stics devplot( curves, markers );
    devplot( deviation, plot );

    // draw time zero marker
    // m = a + bt
    double y0 = (-coeffs[0])/coeffs[1];  // time on y-axis
    double x0 = 0.0;
    mspeakswnd::draw_crosshair_marker draw_marker( markers );
    draw_marker( x0, y0, plot );

    // draw regression line
    QVector< QPointF > data;
    double t0 = 0.0;
    double m0 = adportable::polfit::estimate_y( peaks.coeffs(), t0 ); // estimate sqrt(m)

    double t1 = *std::max_element( peaks.x().begin(), peaks.x().end() ); // time
    double m1 = adportable::polfit::estimate_y( peaks.coeffs(), t1 ); // sqrt(m)
    data.push_back( QPointF(m0, t0) );
    data.push_back( QPointF(m1, t1) );
    plot.setAxisScale( QwtPlot::yLeft, -(t1 * 0.05), t1 * 1.05 );
    plot.setAxisScale( QwtPlot::xBottom, m0, m1 * 1.10 );

    mspeakswnd::draw_regression regplot( curves );
    regplot( data, (boost::format( "lap# %d" ) % mode).str(), plot );

    plot.replot();
    plot.zoomer()->setZoomBase( false );

    MainWindow::instance()->selPage( MainWindow::idSelMSPeaks );
}

void
MSPeaksWnd::handleSetData( const QString& formula, const adcontrols::MSPeaks& peaks )
{
    adplot::plot& plot = *plots_[ 1 ];
    std::vector< std::shared_ptr< QwtPlotMarker > >& markers = plotMarkers_[ 1 ];
    std::vector< std::shared_ptr< QwtPlotCurve > >& curves = plotCurves_[ 1 ];

    markers.clear();
    curves.clear();

    // header
    plot.setTitle( adcontrols::ChemicalFormula::formatFormula( formula.toStdString() ) );

    // footer
    std::ostringstream o;
    o << "T(&mu;s) = ";
    const std::vector<double> coeffs = peaks.coeffs();
    o << boost::format( "%.7e + " ) % coeffs[0];
    o << boost::format( "%.7e&sdot;L(m)" ) % coeffs[1];
    for ( size_t i = 2; i < coeffs.size(); ++i )
        o << boost::format( " + %.7e&sdot;L<sup>%d</sup>") % coeffs[i] % i;
    plot.setFooter( o.str() );

    // markers (data points)
    for ( size_t i = 0; i < peaks.x().size(); ++i ) {
        std::shared_ptr< QwtPlotMarker > marker = std::make_shared< QwtPlotMarker >( peaks[ i ].formula().c_str() );
        markers.push_back( marker );
        marker->setValue( peaks.x()[ i ], peaks.y()[ i ] );
		marker->setSymbol( new QwtSymbol( QwtSymbol::Style( QwtSymbol::XCross ), Qt::NoBrush, QPen( Qt::darkMagenta ), QSize(5, 5) ) );
        marker->attach( &plot );
    }

    // deviation
    QVector< QPointF > deviation;
    for ( size_t i = 0; i < peaks.x().size(); ++i ) {
		double x = peaks.x()[i];
        double y = peaks.y()[i] - adportable::polfit::estimate_y( peaks.coeffs(), x );
        deviation.push_back( QPointF( x, y * 1000 ) );
    }
    mspeakswnd::draw_stics devplot( curves, markers );
    devplot( deviation, plot );
    
    double x0 = 0.0;
    double y0 = coeffs[ 0 ];
    mspeakswnd::draw_crosshair_marker draw_marker( markers );
    draw_marker( x0, y0, plot );

    // draw regression line
    QVector< QPointF > data;
    if ( y0 > 0 ) {
        x0 = ( - peaks.coeffs()[ 0 ] ) / peaks.coeffs()[ 1 ];
        y0 = 0.0;
    }
    double x1 = *std::max_element( peaks.x().begin(), peaks.x().end() );
    double y1 = adportable::polfit::estimate_y( peaks.coeffs(), x1 );
    data.push_back( QPointF(x0, y0) );
    data.push_back( QPointF(x1, y1) );
    plot.setAxisScale( QwtPlot::yLeft, -(y1 * 0.05), y1 * 1.05 );

    mspeakswnd::draw_regression regplot( curves );
    regplot( data, adcontrols::ChemicalFormula::formatFormula( formula.toStdString() ), plot );
	plot.replot();
	plot.zoomer()->setZoomBase( false );

    MainWindow::instance()->selPage( MainWindow::idSelMSPeaks );
}

void
MSPeaksWnd::handlePrintCurrentView( const QString& pdfname )
{
	// A4 := 210mm x 297mm (8.27 x 11.69 inch)
	QSizeF sizeMM( 260, 160 ); // 260x160mm 

    int resolution = 300;
	const double mmToInch = 1.0 / 25.4;
    const QSizeF size = sizeMM * mmToInch * resolution;
    double margin_left = 0.2 /* inch */ * resolution;
    //double margin_right = ( 8.27 - 0.2 ) * resolution;
    //double margin_center = (margin_right - margin_left) / 2 + margin_left;

	QPrinter printer;
    printer.setColorMode( QPrinter::Color );
    printer.setPaperSize( QPrinter::A4 );
    printer.setFullPage( false );
	printer.setOrientation( QPrinter::Landscape );
    
    printer.setDocName( "QtPlatz MS Peaks" );
    printer.setOutputFileName( pdfname );
    printer.setResolution( resolution );

    //-------------------- 
    QPainter painter( &printer );
    int n = 0;
    for ( auto& plot: plots_ ) {
        QRectF boundingRect;
        QRectF drawRect( margin_left, 0.0, printer.width() / 2.0, printer.height() );
        if ( n++ & 01 ) {
			drawRect.moveLeft( printer.width() / 2.0 );
            painter.drawText( drawRect, Qt::TextWordWrap, "Relationship between time and flight length", &boundingRect );
        } else {
            painter.drawText( drawRect, Qt::TextWordWrap, "Relationship between time and m/z", &boundingRect );
        }
        QwtPlotRenderer renderer;
        renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasBackground, true );
        renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasFrame, true );
        renderer.setDiscardFlag( QwtPlotRenderer::DiscardBackground, true );
        
        drawRect.setTop( boundingRect.bottom() );
        drawRect.setHeight( size.height() );
        drawRect.setWidth( size.width() / 2 );
        renderer.render( plot.get(), &painter, drawRect ); // render plot
    }
}
