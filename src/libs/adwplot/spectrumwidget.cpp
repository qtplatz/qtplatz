// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include "spectrumwidget.hpp"
#include "zoomer.hpp"
#include "picker.hpp"
#include "plotcurve.hpp"
#include "annotation.hpp"
#include "annotations.hpp"
#include <adcontrols/annotation.hpp>
#include <adcontrols/annotations.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adportable/array_wrapper.hpp>
#include <adportable/float.hpp>
#include <adportable/debug.hpp>
#include <qtwrapper/font.hpp>
#include <qwt_plot_picker.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_picker_machine.h>
#include <qwt_text.h>
#include <boost/format.hpp>
#include <set>

using namespace adwplot;

namespace adwplot {
    namespace spectrumwidget {

        static Qt::GlobalColor color_table[] = {
            Qt::blue          // 0
            , Qt::red           // 1
            , Qt::green         // 2
            , Qt::cyan          // 3
            , Qt::magenta       // 4
            , Qt::yellow        // 5
            , Qt::darkRed       // 6
            , Qt::darkGreen     // 7
            , Qt::darkBlue      // 8
            , Qt::darkCyan      // 9
            , Qt::darkMagenta   // 10
            , Qt::darkYellow    // 11
            , Qt::darkGray      // 12
            , Qt::black         // 13
            , Qt::lightGray      // 14
            , Qt::white          // 15
            , Qt::gray    // 16
            , Qt::transparent    // 17
        };

        class xSeriesData : public QwtSeriesData<QPointF>, boost::noncopyable {
        public:
            virtual ~xSeriesData() {
            }

            xSeriesData( const adcontrols::MassSpectrum& ms
                         , const QRectF& rc
                         , bool axisTime ) : rect_( rc )
                                           , ms_( ms )
                                           , axisTime_( axisTime ) {
            }

            size_t size() const override { 
                if ( ! indecies_.empty() )
                    return indecies_.size();
                return ms_.size();
            }

            QPointF sample( size_t idx ) const override {
                if ( ! indecies_.empty() && idx < indecies_.size() ) // if colored
                    idx = indecies_[ idx ];
                using namespace adcontrols::metric;
                if ( axisTime_ )
                    return QPointF( scale_to<double, micro>( ms_.getTime( idx  )), ms_.getIntensity( idx ) );
                else
                    return QPointF( ms_.getMass( idx ), ms_.getIntensity( idx ) );
            }

            QRectF boundingRect() const override {
                return rect_;
            }

            size_t make_color_index( unsigned char color ) {
                const unsigned char * colors = ms_.getColorArray();
                for ( size_t i = 0; i < ms_.size(); ++i ) {
                    if ( color == colors[i] )
                        indecies_.push_back( i );
                }
                return indecies_.size();
            }

        private:
            QRectF rect_;
            const adcontrols::MassSpectrum& ms_;
            std::vector< size_t > indecies_; // if centroid with color, 
            bool axisTime_;
        };
	
        class TraceData {
        public:
            TraceData( int idx ) : idx_( idx )
                                 , focusedFcn_( -1 ) {
            }
            TraceData( const TraceData& t ) : idx_( t.idx_ ), curves_( t.curves_ ) {   }
            ~TraceData();
            void setData( Dataplot& plot
                          , const std::shared_ptr< adcontrols::MassSpectrum>&
                          , QRectF&, SpectrumWidget::HorizontalAxis, bool yRight );
            void setFocusedFcn( int fcn );
            std::pair<double, double> y_range( double left, double right ) const;

        private:
            void setProfileData( Dataplot& plot, const adcontrols::MassSpectrum& ms, const QRectF&, bool yRight );
            void setCentroidData( Dataplot& plot, const adcontrols::MassSpectrum& ms, const QRectF&, bool yRight );
            void changeFocus( int focusedFcn );

            int idx_;
            int focusedFcn_;
            QRectF rect_;
            bool yRight_;
            std::vector< PlotCurve > curves_;
            std::shared_ptr< adcontrols::MassSpectrum > pSpectrum_;
			bool isTimeAxis_;
        };
        
    } // namespace spectrumwidget

    struct SpectrumWidgetImpl {
        SpectrumWidgetImpl() : autoAnnotation_( true )
                             , isTimeAxis_( false ) {
        }
        std::weak_ptr< adcontrols::MassSpectrum > centroid_;  // for annotation
        std::vector< Annotation > annotations_;
        std::vector< spectrumwidget::TraceData > traces_;
        bool autoAnnotation_;
        bool isTimeAxis_;

        void clear();
        void update_annotations( Dataplot&, const std::pair<double, double>& );
		void clear_annotations();

        QwtText tracker1( const QPointF& );
        QwtText tracker2( const QPointF&, const QPointF& );
    };

} // namespace adwplot

SpectrumWidget::~SpectrumWidget()
{
    delete impl_;
}

SpectrumWidget::SpectrumWidget(QWidget *parent) : Dataplot(parent)
                                                , impl_( new SpectrumWidgetImpl )
                                                , autoYZoom_( true ) 
                                                , keepZoomed_( true )
                                                , haxis_( HorizontalAxisMass )
                                                , focusedFcn_( -1 ) // no focus
{
    //zoomer2_.reset();
	zoomer1_->autoYScale( autoYZoom_ );

    using namespace std::placeholders;
    zoomer1_->tracker1( std::bind( &SpectrumWidgetImpl::tracker1, impl_, _1 ) );
    zoomer1_->tracker2( std::bind( &SpectrumWidgetImpl::tracker2, impl_, _1, _2 ) );

	if ( picker_ ) {
		connect( picker_.get(), SIGNAL( moved( const QPointF& ) ), this, SLOT( moved( const QPointF& ) ) );
		connect( picker_.get(), SIGNAL( selected( const QRectF& ) ), this, SLOT( selected( const QRectF& ) ) );
		picker_->setEnabled( true );
	}

    setAxisTitle(QwtPlot::xBottom, QwtText( "<i>m/z</i>", QwtText::RichText ) );
    setAxisTitle(QwtPlot::yLeft, QwtText( "Intensity" ) );

    // -----------
    QFont font;
    qtwrapper::font::setFont( font, qtwrapper::fontSizeSmall, qtwrapper::fontAxisLabel );

    // handle zoom rect by this
    if ( zoomer1_ ) {
        connect( zoomer1_.get(), SIGNAL( zoom_override( QRectF& ) ), this, SLOT( override_zoom_rect( QRectF& ) ) );
		QwtPlotZoomer * p = zoomer1_.get();
		connect( p, SIGNAL( zoomed( const QRectF& ) ), this, SLOT( zoomed( const QRectF& ) ) );
	}
	/*
	if ( picker_ ) {
		connect( picker_.get(), SIGNAL( moved( const QPointF& ) ), this, SLOT( moved( const QPointF& ) ) );
		connect( picker_.get(), SIGNAL( selected( const QRectF& ) ), this, SLOT( selected( const QRectF& ) ) );
		picker_->setEnabled( true );
	}
	*/
}

void
SpectrumWidget::update_annotation()
{
    QRectF rc = zoomRect();
    impl_->update_annotations( *this, std::make_pair( rc.left(), rc.right() ) );
	replot();
}

void
SpectrumWidget::override_zoom_rect( QRectF& rc )
{
    if ( autoYZoom_ ) {
        using spectrumwidget::TraceData;
        double bottom = rc.bottom();
        double top = rc.top();
        for ( const TraceData& trace: impl_->traces_ ) {
            std::pair<double, double> y = trace.y_range( rc.left(), rc.right() );
            if ( bottom > y.first )
                bottom = y.first;
            if ( top < y.second )
                top = y.second; // rc.setTop( y.second );
        }
        rc.setBottom( bottom );
        rc.setTop( top + ( top - bottom ) * 0.12 );  // increase 12% for annotation
    }
}

void
SpectrumWidget::setKeepZoomed( bool value )
{
    keepZoomed_ = value;
}

void
SpectrumWidget::zoomed( const QRectF& rect )
{
    impl_->update_annotations( *this, std::make_pair<>( rect.left(), rect.right() ) );
}

void
SpectrumWidget::moved( const QPointF& pos )
{
	(void)pos;
}

void
SpectrumWidget::selected( const QPointF& pos )
{
	emit onSelected( pos );
}

void
SpectrumWidget::selected( const QRectF& rect )
{
	emit onSelected( rect );
}

void
SpectrumWidget::clear()
{
    impl_->clear();
    zoomer1_->setZoomBase();
}

void
SpectrumWidget::setAutoAnnotation( bool enable )
{
    impl_->autoAnnotation_ = enable;
}

bool
SpectrumWidget::autoAnnotation() const
{
    return impl_->autoAnnotation_;
}

void
SpectrumWidget::setAxis( HorizontalAxis haxis )
{
    clear();
    haxis_ = haxis;
    impl_->isTimeAxis_ = haxis == HorizontalAxisTime;
    setAxisTitle(QwtPlot::xBottom, QwtText( haxis_ == HorizontalAxisMass ? "<i>m/z</i>" : "Time[&mu;s]", QwtText::RichText) );
}

void
SpectrumWidget::setData( const std::shared_ptr< adcontrols::MassSpectrum >& ptr, int idx, bool yaxis2 )
{
    using spectrumwidget::TraceData;

    while ( int( impl_->traces_.size() ) <= idx ) 
		impl_->traces_.push_back( TraceData( static_cast<int>(impl_->traces_.size()) ) );

    TraceData& trace = impl_->traces_[ idx ];

    QRectF rect;
    trace.setData( *this, ptr, rect, haxis_, yaxis2 );

	QRectF z = zoomer1_->zoomRect(); // current (:= previous) zoom

    setAxisScale( QwtPlot::xBottom, rect.left(), rect.right() );
    setAxisScale( yaxis2 ? QwtPlot::yRight : QwtPlot::yLeft, rect.top(), rect.bottom() );
    
    zoomer1_->setZoomBase( true );

    if ( keepZoomed_ ) // if ( ! addedTrace )
        Dataplot::zoom( z ); // push previous rect

    if ( ptr->isCentroid() ) {
        impl_->centroid_ = ptr;
        impl_->clear_annotations();
        impl_->update_annotations( *this, ptr->getAcquisitionMassRange() );
    }

    replot();
}

void
SpectrumWidget::setFocusedFcn( int fcn )
{
    for ( auto& trace: impl_->traces_ )
		trace.setFocusedFcn( fcn );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

using namespace adwplot::spectrumwidget;

TraceData::~TraceData()
{
    curves_.clear();
}

void
TraceData::setProfileData( Dataplot& plot, const adcontrols::MassSpectrum& ms, const QRectF& rect, bool yRight )
{
    adcontrols::segment_wrapper< const adcontrols::MassSpectrum > segments( ms );
    int fcn = 0;
    for ( auto& seg: segments ) {
        curves_.push_back( PlotCurve( plot ) );

        PlotCurve &curve = curves_.back();
        QColor color( color_table[ idx_ ] );
        curve.p()->setPen( color );
        curve.p()->setData( new xSeriesData( seg, rect, isTimeAxis_ ) );
        if ( yRight )
            curve.p()->setYAxis( QwtPlot::yRight );
        ++fcn;
    }
}

void
TraceData::changeFocus( int focusedFcn )
{
    int fcn = 0;
    for ( auto& curve: curves_ ) {
        QColor color( color_table[ idx_ ] );
        if ( focusedFcn >= 0 ) {
            if ( focusedFcn != fcn )
                color.setAlpha( 0x20 );
        }
        curve.p()->setPen( color );
        ++fcn;
    }
}

void
TraceData::setCentroidData( Dataplot& plot, const adcontrols::MassSpectrum& _ms, const QRectF& rect, bool yRight )
{
    adcontrols::segment_wrapper< const adcontrols::MassSpectrum > segments( _ms );

    curves_.clear();

    for ( auto& seg: segments ) {
        if ( const unsigned char * colors = seg.getColorArray() ) {
            std::set< unsigned char > color;
            for ( size_t i = 0; i < seg.size(); ++i )
                color.insert( colors[i] );

            for ( const auto& c: color ) {
                xSeriesData * xp = new xSeriesData( seg, rect, isTimeAxis_ );
                if ( xp->make_color_index( c ) ) {
                    curves_.push_back( PlotCurve( plot ) );
                    PlotCurve &curve = curves_.back();
                    curve.p()->setData( xp );
                    curve.p()->setPen( QPen( color_table[ c ] ) );
                    curve.p()->setStyle( QwtPlotCurve::Sticks );
                    if ( yRight )
                        curve.p()->setYAxis( QwtPlot::yRight );
                }
            }

        } else {
            curves_.push_back( PlotCurve( plot ) );
            PlotCurve &curve = curves_.back();
            curve.p()->setPen( QPen( color_table[ 0 ] ) );
            curve.p()->setData( new xSeriesData( seg, rect, isTimeAxis_ ) );
            curve.p()->setStyle( QwtPlotCurve::Sticks );
            if ( yRight )
                curve.p()->setYAxis( QwtPlot::yRight );
        }
    }
}

void
TraceData::setData( Dataplot& plot
                    , const std::shared_ptr< adcontrols::MassSpectrum >& ms
                    , QRectF& rect
                    , SpectrumWidget::HorizontalAxis haxis
                    , bool yRight )
{
    curves_.clear();

    pSpectrum_ = ms;
    isTimeAxis_ = haxis == SpectrumWidget::HorizontalAxisTime;

	double top = adcontrols::segments_helper::max_intensity( *ms );
    double bottom = adcontrols::segments_helper::min_intensity( *ms );
	
	if ( ms->isCentroid() )
		bottom = 0;
	top = top + ( top - bottom ) * 0.12; // add 12% margine for annotation

    if ( isTimeAxis_ ) {

        adcontrols::segment_wrapper< const adcontrols::MassSpectrum > segments( *ms );
        std::pair< double, double > time_range = std::make_pair( std::numeric_limits<double>::max(), 0 );
        for ( auto& m: segments ) {
            std::pair< double, double > range = m.getMSProperty().instTimeRange();
            time_range.first = std::min( time_range.first, range.first );
            time_range.second = std::max( time_range.second, range.second );
        }

        using namespace adcontrols::metric;
        rect.setCoords( scale_to_micro(time_range.first), bottom, scale_to_micro(time_range.second), top );

    } else {

        const std::pair< double, double >& mass_range = ms->getAcquisitionMassRange();
        rect.setCoords( mass_range.first, bottom, mass_range.second, top );

    }
    rect_ = rect;
    yRight_ = yRight;
    if ( ms->isCentroid() ) { // sticked
        setCentroidData( plot, *ms, rect, yRight );
    } else { // Profile
        setProfileData( plot, *ms, rect, yRight );
    }
}

void
TraceData::setFocusedFcn( int fcn )
{
    if ( focusedFcn_ != fcn ) {
        focusedFcn_ = fcn;
        if ( pSpectrum_ ) {
            changeFocus( focusedFcn_ );
        }
    }
}


std::pair< double, double >
TraceData::y_range( double left, double right ) const
{
    double top = 100;
    double bottom = -10;

    // if ( const std::shared_ptr< adcontrols::MassSpectrum > ms = pSpectrum_.lock() ) {
    if ( const std::shared_ptr< adcontrols::MassSpectrum > ms = pSpectrum_ ) {
        adcontrols::segment_wrapper< const adcontrols::MassSpectrum > segments( *ms );
        bool isCentroid = ms->isCentroid();
        for ( auto& seg: segments ) {

            size_t idleft(0), idright(0);

            if ( isTimeAxis_ ) {
                using namespace adcontrols::metric;
                double uleft = scale_to_base(left, micro);
                double uright = scale_to_base(right, micro);

                if ( isCentroid ) {
                    if ( ms->isCentroid() ) {
                        const double * x = seg.getTimeArray();
                        idleft = std::distance( x, std::lower_bound( x, x + seg.size(), uleft ) );
                        idright = std::distance( x, std::lower_bound( x, x + seg.size(), uright ) );
                    }
                } else {
                    // std::pair< double, double > range = seg.getMSProperty().instTimeRange();
                    
                    struct X {
                        static uint32_t index( double interval, uint32_t delay, uint32_t nSamples, double t ) {
                            uint32_t idx = uint32_t( t / interval + 0.5 );
                            if ( idx < delay )
                                return 0;
                            idx -= delay;
                            if ( idx >= nSamples )
                                return nSamples - 1;
                            return idx;
                        }
                    };
                    const adcontrols::MSProperty::SamplingInfo& info = seg.getMSProperty().getSamplingInfo();
                    idleft = X::index( info.fSampInterval(), info.nSamplingDelay, info.nSamples, uleft );
                    idright = X::index( info.fSampInterval(), info.nSamplingDelay, info.nSamples, uright );
                }
            } else {
                // mass axis
				const double * x = seg.getMassArray();
                idleft = std::distance( x, std::lower_bound( x, x + seg.size(), left ) );
                idright = std::distance( x, std::lower_bound( x, x + seg.size(), right ) );
            }

            
            if ( idleft < idright ) {
                const double * y = seg.getIntensityArray();
                
                auto minmax = std::minmax_element( y + idleft, y + idright );
                double min = *minmax.first;
                double max = *minmax.second;

                top = std::max( top, max );
                if ( ! isCentroid ) {
                    bottom = min - (max - min) / 20;
                } else {
                    bottom = -10;
                }
            }
        }
    }
    return std::make_pair<>(bottom, top);
}

void
SpectrumWidgetImpl::clear_annotations()
{
	annotations_.clear();
}

void
SpectrumWidgetImpl::update_annotations( Dataplot& plot
                                       , const std::pair<double, double>& range )
{
    using adportable::array_wrapper;
    using namespace adcontrols::metric;        

	// QRectF zrc = plot.zoomRect();

    typedef std::tuple< size_t, size_t, int, double, double > peak; // fcn, idx, color, mass, intensity
    enum { c_fcn, c_idx, c_color, c_intensity, c_mass };

    if ( auto centroid = centroid_.lock() ) {

        std::vector< peak > peaks;
        adcontrols::segment_wrapper< const adcontrols::MassSpectrum > segments( *centroid );
        
        adcontrols::annotations auto_annotations;
        adcontrols::annotations annotations;

        double max_y = adcontrols::segments_helper::max_intensity( *centroid );
        
        for ( size_t fcn = 0; fcn < segments.size(); ++fcn ) {
            const adcontrols::MassSpectrum& ms = segments[ fcn ];
            const unsigned char * colors = ms.getColorArray();

            size_t beg, end;
            if ( isTimeAxis_ ) {
                array_wrapper< const double > times( ms.getTimeArray(), ms.size() );
                beg = std::distance( times.begin(), std::lower_bound( times.begin(), times.end(), scale_to_base( range.first, micro ) ) );
                end = std::distance( times.begin(), std::lower_bound( times.begin(), times.end(), scale_to_base( range.second, micro) ) );
            } else {
                array_wrapper< const double > masses( ms.getMassArray(), ms.size() );
                beg = std::distance( masses.begin(), std::lower_bound( masses.begin(), masses.end(), range.first ) );
                end = std::distance( masses.begin(), std::lower_bound( masses.begin(), masses.end(), range.second ) );
            }
            
            if ( beg < end ) {
                const adcontrols::annotations& attached = ms.get_annotations();
                std::map< size_t, adcontrols::annotations > marge;
                for ( auto a : attached ) {
                    if ( ( int(beg) <= a.index() && a.index() <= int(end) ) || ( range.first < a.x() && a.x() < range.second ) ) {
                        if ( a.index() >= 0 ) {
							if ( isTimeAxis_ ) {
								a.x( scale_to_micro( ms.getTime( a.index() ) ) );
							} else {
								a.x( ms.getMass( a.index() ) );
							}
							a.y( ms.getIntensity( a.index() ) );
						}
						if ( a.dataFormat() == adcontrols::annotation::dataFormula ) {
							a.text( adcontrols::ChemicalFormula::formatFormula( a.text () ), adcontrols::annotation::dataFormula );
                        }
                        marge[ a.index() ] << a;
                    }
                }
                for ( auto& a : marge ) {
                    // if more than two annotations attached to an index, formula is a priority on spectrum
                    auto it = std::find_if( a.second.begin(), a.second.end(), [] ( const adcontrols::annotation& x ) { return x.dataFormat() == adcontrols::annotation::dataFormula;  } );
                    if ( it == a.second.end() )
                        it = std::find_if( a.second.begin(), a.second.end(), [] ( const adcontrols::annotation& x ) { return x.dataFormat() == adcontrols::annotation::dataText;  } );
                    if ( it != a.second.end() )
                        annotations << *it;
                }

                if ( autoAnnotation_ ) {
                    // generate auto-annotation
                    for ( size_t idx = beg; idx <= end; ++idx ) {
                        if ( std::find_if( attached.begin()
                                           , attached.end()
                                           , [idx]( const adcontrols::annotation& a ){ return a.index() == int(idx); } ) == attached.end() ) {
                            int pri = ms.getIntensity( idx ) / max_y * 1000;
                            if ( colors )
                                pri *= 100;
                            if ( isTimeAxis_ ) {
                                double microseconds = adcontrols::metric::scale_to_micro( ms.getTime( idx ) );
                                adcontrols::annotation annot( ( boost::wformat( L"%.4lf" ) % microseconds ).str()
                                                              , microseconds, ms.getIntensity( idx )
                                                              , int( fcn << 24 | idx ), pri );
                                auto_annotations << annot;
                            } else {
                                adcontrols::annotation annot( ( boost::wformat( L"%.4lf" ) % ms.getMass( idx ) ).str()
                                                              , ms.getMass( idx ), ms.getIntensity( idx ), int( fcn << 24 | idx ), pri );
                                auto_annotations << annot;
                            }
                        }
                    }
                }
            }
        }
        
        auto_annotations.sort();
        annotations.sort();
        
        annotations_.clear();
        Annotations annots(plot, annotations_);
        
        for ( const auto& a: annotations ) {
            QwtText text( QString::fromStdString(a.text()), QwtText::RichText);
            text.setColor( Qt::darkGreen );
            text.setFont( Annotation::font() );
            annots.insert( a.x(), a.y(), text, Qt::AlignTop | Qt::AlignHCenter );
        }
        
        QColor color = Qt::darkGreen;
        QFont font = Annotation::font();
        if ( ! annotations.empty() ) {
            // if user defined annotations were exist
            font.setPointSize( font.pointSize() - 2 );
            color = Qt::gray;
        }
        for ( const auto& a: auto_annotations ) {
            QwtText text( QString::fromStdString(a.text()), QwtText::RichText );
            text.setColor( color );
            text.setFont( font );
			annots.insert( a.x(), a.y(), text, Qt::AlignTop | Qt::AlignHCenter );
        }
    }
}

void
SpectrumWidgetImpl::clear()
{
    centroid_.reset();
    annotations_.clear();
    traces_.clear();
}

QwtText
SpectrumWidgetImpl::tracker1( const QPointF& pos )
{
    if ( isTimeAxis_ ) {
        return QwtText( (boost::format( "%.4f&mu;s" ) % pos.x()).str().c_str(), QwtText::RichText );
    }
    else {
        return QwtText( (boost::format( "<i>m/z=</i>%.4f" ) % pos.x()).str().c_str(), QwtText::RichText );
    }
}

QwtText
SpectrumWidgetImpl::tracker2( const QPointF& p1, const QPointF& pos )
{
    double d = ( pos.x() - p1.x() );

    if ( isTimeAxis_ ) {
        if ( std::abs( d ) < 1.0 )
            return QwtText( (boost::format( "%.4f&mu;s (&delta;=%gns)" ) % pos.x() % (d * 1000)).str().c_str(), QwtText::RichText );
        else
            return QwtText( (boost::format( "%.4f&mu;s (&delta;=%g&mu;s)" ) % pos.x() % d).str().c_str(), QwtText::RichText );
    }
    else {
        if ( std::abs( d ) < 1.0 )
            return QwtText( (boost::format( "<i>m/z=</i>%.4f (&delta;=%gmDa)" ) % pos.x() % (d * 1000)).str().c_str(), QwtText::RichText );
        else
            return QwtText( (boost::format( "<i>m/z=</i>%.4f (&delta;=%gDa)" ) % pos.x() % d).str().c_str(), QwtText::RichText );
    }
}
