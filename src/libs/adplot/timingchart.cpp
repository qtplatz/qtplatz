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

#include "timingchart.hpp"
#include "zoomer.hpp"
#include <adcontrols/metric/prefix.hpp>
#include <qwt_legend.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_directpainter.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_marker.h>
#include <qwt_symbol.h>
#include <qwt_scale_draw.h>
#include <qwt_scale_engine.h>
#include <qwt_scale_widget.h>
#include <qwt_wheel.h>
#include <QApplication>
#include <QEvent>
#include <QResizeEvent>
#include <array>
#include <memory>

namespace adplot {

    class TimingChartPicker : public QObject {
        Q_OBJECT
    public:
        TimingChartPicker( TimingChart * plot );

        bool eventFilter( QObject *, QEvent * ) override;
        bool event( QEvent * ) override;
        TimingChart * plot() { return qobject_cast< TimingChart * >( parent() ); }
        const TimingChart * plot() const { return qobject_cast< TimingChart * >( parent() ); }
    };

    class pulseMarker {
    public:
        ~pulseMarker() {
            delete marker1_;
            delete marker2_;
        }

        pulseMarker() : marker1_( new QwtPlotMarker )
                      , marker2_( new QwtPlotMarker ) {

        }

        void detach() {
            marker1_->detach();
            marker2_->detach();
        }

        void attach( QwtPlot * plot ) {
            marker1_->setLineStyle( QwtPlotMarker::VLine );
            marker1_->setLabelAlignment( Qt::AlignLeft );

            marker2_->setLinePen( QColor( 0x00, 0x00, 0xff, 0x80 ), 0, Qt::DashLine );
            marker2_->setLineStyle( QwtPlotMarker::VLine );
            marker2_->setLabelAlignment( Qt::AlignRight );

            marker1_->attach( plot );
            marker2_->attach( plot );
        }
        
        void setValue( const QPointF& p1, const QPointF& p2 ) {

            marker1_->setValue( p1.x(), p1.y() );
            marker2_->setValue( p2.x(), p2.y() );

            QwtText label1( QString( "%1&mu;s" ).arg( QString::number( p1.x(), 'f', 3 ) ), QwtText::RichText );
            QwtText label2( QString( "%1&mu;s\n(%2&mu;s)" ).arg( QString::number( p2.x(), 'f', 3 )
                                                                , QString::number( p2.x() - p1.x(), 'f', 3 ) ), QwtText::RichText );
            
            marker1_->setLabel( label1 );
            marker2_->setLabel( label2 );
        }

        QwtPlotMarker *marker1_;
        QwtPlotMarker *marker2_;
    };

    class TimingChart::impl {
    public:
        impl( TimingChart * p ) : this_( p ) 
                                , selectedCurve_( 0 )
                                , selectedPoint_( -1 )
                                , pulseMarker_( std::make_shared< pulseMarker >() )
            {}

        void addPulse( const timingchart::pulse& );

        std::shared_ptr< pulseMarker > pulseMarker_;
        std::vector< std::pair< timingchart::pulse, std::shared_ptr< QwtPlotCurve > > > pulses_;
        std::vector< std::shared_ptr< QwtPlotMarker > > markers_;
        QwtPlotCurve * selectedCurve_;
        int selectedPoint_;
    private:
        TimingChart * this_;
    };

}

using namespace adplot;

TimingChart::~TimingChart()
{
    delete impl_;
}

TimingChart::TimingChart(QWidget *parent) : plot(parent)
                                          , impl_( new impl( this ) )
{
    setTitle( QString( "Timing Chart" ) );

    new TimingChartPicker( this );
    // canvas()->installEventFilter( this );

    QwtPlotGrid *grid = new QwtPlotGrid;
    grid->setMajorPen( Qt::white, 0, Qt::DotLine );
    grid->attach( this );

    setAxisScale( QwtPlot::xBottom, -100.0, 1000.0 );
    setAxisScale( QwtPlot::yLeft, -5.0, 80.0 );
    setAxisTitle( QwtPlot::xBottom, QwtText( "&mu;s", QwtText::RichText ) );
    
    plot::zoomer()->setZoomBase();

    QwtScaleDraw *sd = axisScaleDraw( QwtPlot::yLeft );
    sd->setMinimumExtent( sd->extent( axisWidget( QwtPlot::yLeft )->font() ) );
    plotLayout()->setAlignCanvasToScales( true );

    auto legend = new QwtLegend();
    legend->setDefaultItemMode( QwtLegendData::Checkable );
    insertLegend( legend, QwtPlot::RightLegend );
    connect( legend, &QwtLegend::checked, [=] ( const QVariant& item, bool on ){ 
            if ( auto plotItem = infoToItem( item ) )
                plotItem->setVisible( on );
        });

    // time zero marker
    if ( auto marker = std::make_shared< QwtPlotMarker >() ) {
        impl_->markers_.push_back( marker );
        marker->setLineStyle( QwtPlotMarker::VLine );
        marker->setLinePen( QColor( 0xff, 0, 0, 0x40 ), 0, Qt::DashLine );
        marker->setValue( 0, 0 );
        marker->attach( this );
    }
}


TimingChart&
TimingChart::operator << ( const timingchart::pulse& pulse )
{
    impl_->addPulse( pulse );
    replot();
    return *this;
}

const timingchart::pulse&
TimingChart::operator [] ( int idx ) const
{
    return impl_->pulses_[ idx ].first;
}

size_t
TimingChart::size() const
{
    return impl_->pulses_.size();
}

void
TimingChart::clear()
{
    impl_->pulses_.clear();
    impl_->markers_.clear();
    impl_->pulseMarker_->detach();
}

// Move the selected point
void
TimingChart::move( const QPoint &pos )
{
    if ( !impl_->selectedCurve_ )
        return;

    QVector< QPointF > samples;
    
    for ( int i = 0; i < impl_->selectedCurve_->dataSize(); ++i )
        samples.push_back( impl_->selectedCurve_->sample( i ) );

    double x = this->invTransform( impl_->selectedCurve_->xAxis(), pos.x() );
    double dx = x - samples[ 0 ].x();

    if ( impl_->selectedPoint_ == 0 ) {

        for ( auto& sample : samples )
            sample.setX( sample.x() + dx );

    }
    else {

        samples[ impl_->selectedPoint_ ].setX( x );

    }

    impl_->selectedCurve_->setSamples( samples );
    impl_->pulseMarker_->setValue( samples[ 0 ], samples[ 1 ] );

    /*
       Enable QwtPlotCanvas::ImmediatePaint, so that the canvas has been
       updated before we paint the cursor on it.
     */
    if ( QwtPlotCanvas *plotCanvas = qobject_cast<QwtPlotCanvas *>(canvas()) ) {
        plotCanvas->setPaintAttribute( QwtPlotCanvas::ImmediatePaint, true );
        replot();
        plotCanvas->setPaintAttribute( QwtPlotCanvas::ImmediatePaint, false );
    }

    showCursor( true );
    
    for ( auto& pair : impl_->pulses_ ) {
        if ( pair.second.get() == impl_->selectedCurve_ ) {
            pair.first.delay( samples[ 0 ].x() );
            pair.first.duration( samples[ 1 ].x() - samples[ 0 ].x() );
            emit valueChanged( pair.first );
        }
    }
}

// Select the point at a position. If there is no point
// deselect the selected point

void
TimingChart::select( const QPoint &pos )
{
    QwtPlotCurve *curve = nullptr;
    double dist = 10e10;
    int index = -1;

    for ( auto& item : itemList() ) {

        if ( (item)->rtti() == QwtPlotItem::Rtti_PlotCurve )   {

            QwtPlotCurve *c = static_cast<QwtPlotCurve *>(item);

            double d;
            int idx = c->closestPoint( pos, &d );
            if ( d < dist )   {
                curve = c;
                index = idx;
                dist = d;
            }
        }
    }

    showCursor( false );

    bool selected = impl_->selectedCurve_ != nullptr;

    impl_->selectedCurve_ = nullptr;
    impl_->selectedPoint_ = -1;

    if ( !selected && ( curve && dist < 10 ) ) { // 10 pixels tolerance

        impl_->selectedCurve_ = curve;
        impl_->selectedPoint_ = index;

        showCursor( true );

        auto s1 = impl_->selectedCurve_->sample( 0 );
        auto s2 = impl_->selectedCurve_->sample( 1 );
        impl_->pulseMarker_->setValue( s1, s2 );
        impl_->pulseMarker_->attach( this );
        replot();
    }
    else {
        impl_->pulseMarker_->detach();
        replot();
    }
}

// Hightlight the selected point
void
TimingChart::showCursor( bool showIt )
{
    if ( !impl_->selectedCurve_ )
        return;

    QwtSymbol *symbol = const_cast<QwtSymbol *>( impl_->selectedCurve_->symbol() );

    const QBrush brush = symbol->brush();
    if ( showIt )
        symbol->setBrush( symbol->brush().color().dark( 180 ) );

    QwtPlotDirectPainter directPainter;
    directPainter.drawSeries( impl_->selectedCurve_, impl_->selectedPoint_, impl_->selectedPoint_ );

    if ( showIt )
        symbol->setBrush( brush ); // reset brush
}


////////////////////////

/////////////////////////////
void
TimingChart::impl::addPulse( const timingchart::pulse& p )
{
    timingchart::pulse pulse( p );

    if ( pulse.uniqId() == -1 )
        pulse.uniqId( int( pulses_.size() ) );

    double base = pulses_.size() * 10;
    
    if ( auto marker = std::make_shared< QwtPlotMarker >() ) {
        markers_.push_back( marker );
        marker->setLineStyle( QwtPlotMarker::HLine );
        marker->setLinePen( QColor( 0x6c, 0x7b, 0x8b, 0x80 ), 0, Qt::SolidLine ); // slate gray
        marker->setValue( 0, base );
        marker->setLabel( pulse.name() );
        marker->setLabelAlignment( Qt::AlignRight | Qt::AlignBottom );
        marker->attach( this_ );
    }

    if ( auto curve = std::make_shared< QwtPlotCurve >() ) {

        pulses_.push_back( std::make_pair( pulse, curve ) );

        static QColor colors[] = { Qt::green, Qt::red, Qt::darkGreen, Qt::blue, Qt::magenta, Qt::darkCyan };
        int idx = pulse.uniqId() % (sizeof( colors ) / sizeof( colors[ 0 ] ));
        curve->setPen( colors[ idx ], 2.0 );
        curve->setSymbol( new QwtSymbol( QwtSymbol::Triangle,  Qt::gray, colors[ idx ], QSize( 8, 8 ) ) );

        QVector< QPointF > line;
        line.push_back( QPointF( pulse.delay(), base ) );
        line.push_back( QPointF( pulse.delay() + pulse.duration(), base ) );
    
        curve->setSamples( line );
        curve->setTitle( pulse.name() );
        curve->setVisible( true );

        curve->attach( this_ );
    }

}

////////////////////////////
TimingChartPicker::TimingChartPicker( TimingChart * plot ) : QObject( plot )
{
    QwtPlotCanvas *canvas = qobject_cast<QwtPlotCanvas *>( plot->canvas() );

    canvas->installEventFilter( this );
    canvas->setFocusPolicy( Qt::StrongFocus );
    canvas->setCursor( Qt::PointingHandCursor );
    canvas->setFocusIndicator( QwtPlotCanvas::ItemFocusIndicator );
    canvas->setFocus();
}

bool
TimingChartPicker::event( QEvent *ev )
{
    if ( ev->type() == QEvent::User )
        plot()->showCursor( true );
    return QObject::event( ev );
}

bool
TimingChartPicker::eventFilter( QObject * object, QEvent * event )
{
    if ( plot() == nullptr || object != plot()->canvas() ) {
        return false;
    }

    switch( event->type() )  {
    case QEvent::FocusIn:
        plot()->showCursor( true );
        break;
    case QEvent::FocusOut: 
        plot()->showCursor( false );
        break;
    case QEvent::Paint:
        QApplication::postEvent( this, new QEvent( QEvent::User ) );
        break;
    case QEvent::MouseButtonPress:
        if ( const QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event) ) {
            plot()->select( mouseEvent->pos() );
            //return true;
        }
        break;
    case QEvent::MouseMove:
        if ( const QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event) ) {
            plot()->move( mouseEvent->pos() );
            //return true;
        }
        break;
    default:
        break;
    }
    return QObject::eventFilter( object, event );
}

////////////////////////////

namespace adplot { namespace timingchart {

pulse::pulse( double delay
              , double duration
              , const QString& name
              , int uniqId ) : delay_( delay )
                             , duration_( duration )
                             , name_( name )
                             , uniqId_( uniqId )
{
}

pulse::pulse( const pulse& t ) : delay_( t.delay_ )
                               , duration_( t.duration_ )
                               , name_( t.name_ )
                               , uniqId_( t.uniqId_ )
{
}

double
pulse::delay( bool microseconds ) const
{
    return microseconds ? 
        adcontrols::metric::scale_to_micro( delay_ ) : delay_;
}

void
pulse::delay( double value, bool microseconds )
{
    delay_ = microseconds ? adcontrols::metric::scale_to_base( value, adcontrols::metric::prefix::micro ) : value;
}

double
pulse::duration( bool microseconds ) const
{
    return microseconds ? 
        adcontrols::metric::scale_to_micro( duration_ ) : duration_;
}

void
pulse::duration( double value, bool microseconds )
{
    duration_ = microseconds ? adcontrols::metric::scale_to_base( value, adcontrols::metric::prefix::micro ) : value;
}

const QString&
pulse::name() const
{
    return name_;
}

void
pulse::name( const QString& value )
{
    name_ = value;
}

int
pulse::uniqId() const
{
    return uniqId_;
}

void
pulse::uniqId( int id )
{
    uniqId_ = id;
}

} // namespace timingchart
} // namespace adplot

#include "timingchart.moc"
