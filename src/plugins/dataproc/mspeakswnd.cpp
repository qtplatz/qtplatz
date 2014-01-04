/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include <adwplot/dataplot.hpp>
#include <adcontrols/mspeaks.hpp>
#include <adcontrols/mspeak.hpp>
#include <adportable/polfit.hpp>
#include <qwt_scale_widget.h>
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

using namespace dataproc;

MSPeaksWnd::MSPeaksWnd(QWidget *parent) : QWidget(parent)
{
    plots_.push_back( std::make_shared< adwplot::Dataplot >() );
    plots_.push_back( std::make_shared< adwplot::Dataplot >() );
    
    plotMarkers_.resize( plots_.size() );
    plotCurves_.resize( plots_.size() );

    QwtText text_haxis( "time (&mu;s)", QwtText::RichText );
    QFont font = text_haxis.font();
	font.setFamily( "Verdana" );
	font.setBold( true );
	font.setItalic( true );
	font.setPointSize( 9 );
    text_haxis.setFont( font );

    do {
        adwplot::Dataplot& plot = *plots_[0];

        plot.setAxisTitle( QwtPlot::xBottom, text_haxis );
        
		QwtText vaxis( "&radic;<span style=\"text-decoration: overline\">&nbsp;<i>m/z</i></span>", QwtText::RichText );
        vaxis.setFont( font );
        plot.setAxisTitle( QwtPlot::yLeft, vaxis );
        if ( QwtPlotLegendItem * legendItem = new QwtPlotLegendItem ) 
            legendItem->attach( &plot );
    } while(0);

    do {
        adwplot::Dataplot& plot = *plots_[1];

        plot.setAxisTitle( QwtPlot::xBottom, text_haxis );
        
		QwtText vaxis( "flight length (m)", QwtText::RichText );
        vaxis.setFont( font );
        plot.setAxisTitle( QwtPlot::yLeft, vaxis );
        if ( QwtPlotLegendItem * legendItem = new QwtPlotLegendItem ) 
            legendItem->attach( &plot );
    } while(0);

    for ( auto& plot: plots_ ) {
        plot->setMinimumHeight( 40 );
        plot->setMinimumWidth( 40 );
        plot->axisAutoScale( QwtPlot::xBottom );
        plot->axisAutoScale( QwtPlot::yLeft );
    }

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
    adwplot::Dataplot& plot = *plots_[ 0 ];
    std::vector< std::shared_ptr< QwtPlotMarker > >& markers = plotMarkers_[ 0 ];
    std::shared_ptr< QwtPlotCurve >& curve = plotCurves_[ 0 ];

    markers.clear();
    curve.reset();

    std::ostringstream o;
    o << "mode: " << mode << "; ";
    o << "&radic;<span style=\"text-decoration: overline\">&nbsp;<i>m/z</i></span> = ";
    const std::vector<double> coeffs = peaks.coeffs();
    o << boost::format( "%.7e + " ) % coeffs[0];
    o << boost::format( "%.7e&sdot;t" ) % coeffs[1];
    for ( size_t i = 2; i < coeffs.size(); ++i )
        o << boost::format( " + %.7e&sdot;t<sup>%d</sup>") % coeffs[i] % i;
    plot.setTitle( o.str() );

    for ( size_t i = 0; i < peaks.x().size(); ++i ) {
        std::shared_ptr< QwtPlotMarker > marker = std::make_shared< QwtPlotMarker >( peaks[ i ].formula().c_str() );
        markers.push_back( marker );
        marker->setLabel( QwtText( peaks[ i ].formula().c_str() ) );
        marker->setLabelAlignment( Qt::AlignRight | Qt::AlignBottom );
        marker->setValue( peaks.x()[ i ], peaks.y()[ i ] );
		marker->setSymbol( new QwtSymbol( QwtSymbol::Style( QwtSymbol::Cross ), Qt::NoBrush, QPen( Qt::darkMagenta ), QSize(5, 5) ) );
        marker->attach( &plot );
    }
    // m = a + bt
    double t0 = (-coeffs[0])/coeffs[1];
    double t1 = *std::max_element( peaks.x().begin(), peaks.x().end() );
    double m1 = adportable::polfit::estimate_y( peaks.coeffs(), t1 );
    QVector<QPointF> data;
    data.push_back( QPointF(t0, 0.0) );
    data.push_back( QPointF(t1, m1) );

    curve = std::make_shared< QwtPlotCurve >();
	curve->setPen( Qt::green );
    curve->setLegendAttribute( QwtPlotCurve::LegendShowLine );
    curve->setYAxis( QwtPlot::yLeft );
    curve->attach( &plot );
    curve->setSamples( data );

    plot.replot();

    MainWindow::instance()->actionSelMSPeaks();
}

void
MSPeaksWnd::handleSetData( const QString& formula, const adcontrols::MSPeaks& peaks )
{
    adwplot::Dataplot& plot = *plots_[ 1 ];
    std::vector< std::shared_ptr< QwtPlotMarker > >& markers = plotMarkers_[ 1 ];
    std::shared_ptr< QwtPlotCurve >& curve = plotCurves_[ 1 ];

    markers.clear();
    curve.reset();

    std::ostringstream o;
	o << formula.toStdString() << "; ";
    o << "length (m) = ";
    const std::vector<double> coeffs = peaks.coeffs();
    o << boost::format( "%.7e + " ) % coeffs[0];
    o << boost::format( "%.7e&sdot;t" ) % coeffs[1];
    for ( size_t i = 2; i < coeffs.size(); ++i )
        o << boost::format( " + %.7e&sdot;t<sup>%d</sup>") % coeffs[i] % i;
    plot.setTitle( o.str() );

    for ( size_t i = 0; i < peaks.x().size(); ++i ) {
        std::shared_ptr< QwtPlotMarker > marker = std::make_shared< QwtPlotMarker >( peaks[ i ].formula().c_str() );
        markers.push_back( marker );
        marker->setValue( peaks.x()[ i ], peaks.y()[ i ] );
		marker->setSymbol( new QwtSymbol( QwtSymbol::Style( QwtSymbol::Cross ), Qt::NoBrush, QPen( Qt::darkMagenta ), QSize(5, 5) ) );
        marker->attach( &plot );
    }

    double t0 = (-coeffs[0])/coeffs[1];
    double x1 = *std::max_element( peaks.x().begin(), peaks.x().end() );
    double y1 = adportable::polfit::estimate_y( peaks.coeffs(), x1 );
    QVector<QPointF> data;
    data.push_back( QPointF(t0, 0.0) );
    data.push_back( QPointF(x1, y1) );

    curve = std::make_shared< QwtPlotCurve >( formula );
	curve->setPen( Qt::green );
    curve->setLegendAttribute( QwtPlotCurve::LegendShowLine );
    curve->setYAxis( QwtPlot::yLeft );
    curve->attach( &plot );
    curve->setSamples( data );
	plot.replot();

    MainWindow::instance()->actionSelMSPeaks();
}

void
MSPeaksWnd::handlePrintCurrentView( const QString& pdfname )
{
	// A4 := 210mm x 297mm (8.27 x 11.69 inch)
    //QSizeF sizeMM( 180, 80 );
	QSizeF sizeMM( 260, 160 );

    int resolution = 300;
	const double mmToInch = 1.0 / 25.4;
    const QSizeF size = sizeMM * mmToInch * resolution;
    double margin_left = 0.2 /* inch */ * resolution;

	QPrinter printer;
    printer.setColorMode( QPrinter::Color );
    printer.setPaperSize( QPrinter::A4 );
    printer.setFullPage( false );
	printer.setOrientation( QPrinter::Landscape );
    
    printer.setDocName( "QtPlatz MS Peaks" );
    printer.setOutputFileName( pdfname );
    // printer.setOutputFormat( QPrinter::PdfFormat );
    printer.setResolution( resolution );

    QPainter painter( &printer );
    
    QRectF boundingRect;
    QRectF drawRect( margin_left, 0.0, printer.width(), (18.0/72)*resolution );
    
    painter.drawText( drawRect, Qt::TextWordWrap, "MS Peaks", &boundingRect );
    
    QwtPlotRenderer renderer;
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasBackground, true );
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasFrame, true );
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardBackground, true );
    
    drawRect.setTop( boundingRect.bottom() );
    drawRect.setHeight( size.height() );
    drawRect.setWidth( size.width() );
    renderer.render( plots_[ 0 ].get(), &painter, drawRect ); // render plot 0
}
