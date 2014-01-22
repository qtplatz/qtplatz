/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#include "mscalibspectrawnd.hpp"
#include "assign_peaks.hpp"
#include "assign_masses.hpp"
#include "mainwindow.hpp"
#include "dataprocessor.hpp"
#include "dataprochandler.hpp"
#include "sessionmanager.hpp"
#include <adcontrols/computemass.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/msassignedmass.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/mscalibration.hpp>
#include <adcontrols/mscalibrateresult.hpp>
#include <adcontrols/mscalibratemethod.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/descriptions.hpp>
#include <adcontrols/description.hpp>
#include <adwplot/spectrumwidget.hpp>
#include <adportable/configuration.hpp>
#include <adportable/debug.hpp>
#include <adportable/array_wrapper.hpp>
#include <adportable/polfit.hpp>
#include <adportable/float.hpp>
#include <adportable/utf.hpp>
#include <adplugin/manager.hpp>
#include <adplugin/widget_factory.hpp>
#include <adplugin/lifecycle.hpp>
#include <adplugin/lifecycleaccessor.hpp>
#include <adutils/processeddata.hpp>
#include <portfolio/portfolio.hpp>
#include <portfolio/folder.hpp>
#include <portfolio/folium.hpp>
#include <coreplugin/minisplitter.h>
#include <qwt_legend.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_legenditem.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_renderer.h>
#include <qwt_scale_widget.h>
#include <qwt_symbol.h>
#include <qwt_scale_engine.h>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QPrinter>
#include <boost/format.hpp>
#include <algorithm>
#include <cmath>
#include <tuple>

namespace dataproc {

    namespace internal {

        struct time_length {
            bool enable;
            double time;
            double length;
            time_length( bool e, double t, double l ) : enable( e ), time(t), length(l) {
            }
        };

		class SeriesData : public std::enable_shared_from_this< SeriesData >
                         , boost::noncopyable {
            std::vector< time_length > d_; // enable, time, length
            std::wstring formula_;
            double exactMass_;
            QRectF rect_;
            bool active_;
            std::vector< double > coeffs_;

        public:
            virtual ~SeriesData() {}
            SeriesData( const std::wstring& formula, double exactMass ) : formula_( formula )
                                                                        , exactMass_( exactMass )
                                                                        , active_( false ) {
            }
            const std::wstring& formula() const { return formula_; }
            inline bool active() const { return active_; }
			inline const std::vector<double>& coeffs() const { return coeffs_; }
            inline double intercept() const { return coeffs_.empty() ? 0 : coeffs_[ 0 ]; }
            inline double slope() const { return coeffs_.empty() ? 0 : coeffs_[ 1 ]; }
            inline double exactMass() const { return exactMass_; }

            size_t size() { return d_.size(); }
            QPointF sample( size_t idx ) const {
				return QPointF( d_[ idx ].length, d_[ idx ].time ); // std::get<2>(d_[ idx ]), std::get<1>(d_[ idx ]) ); // (length, time)
			}
            QRectF boundingRect() const {
				return rect_;
			}  

            // local member
            void clear() {  d_.clear(); }
            SeriesData& operator << ( time_length d ) {
                d_.push_back( d );
                rect_.setLeft( std::min( d.length,   rect_.left() ) );
                rect_.setRight( std::max( d.length,  rect_.right() ) );
                rect_.setTop( std::min( d.time,    rect_.top() ) );
                rect_.setBottom( std::max( d.time, rect_.bottom() ) );
                return *this;
            }
            
            bool polfit( double& a, double& b ) {
                std::vector< double > time, length;
                for ( auto t: d_ ){
                    if ( t.enable ) { 
                        time.push_back( t.time );   // time
                        length.push_back( t.length ); // length
                    }
                }
                coeffs_.clear();
                if ( ( active_ = adportable::polfit::fit( length.data(), time.data(), time.size(), 2, coeffs_ ) ) ) {
                    a = coeffs_[ 0 ];
                    b = coeffs_[ 1 ];
                    return true;
                }
                return false;
            }
        };

        class xSeriesData : public QwtSeriesData< QPointF > {
            std::shared_ptr< SeriesData > p_;
		public:
            xSeriesData( std::shared_ptr< SeriesData >& p ) : p_( p ) {
            }

            size_t size() const override {  
				return p_->size(); 
			}
            QPointF sample( size_t idx ) const override { 
				return p_->sample( idx );
			}
            QRectF boundingRect() const override {
				return p_->boundingRect(); 
			}
        };
    }
}

using namespace dataproc;

MSCalibSpectraWnd::~MSCalibSpectraWnd()
{
	// detach all object attached to QwtPlot
	slopeMarkers_.clear();
    interceptMarkers_.clear();
    markers_.clear();
    plotCurves_.clear();
    plotRegressions_.clear();

    if ( slopePlotCurve_ )
        slopePlotCurve_->detach();

    if ( interceptPlotCurve_ )
        interceptPlotCurve_->detach();

    if ( regressionCurve_ )
        regressionCurve_->detach();
}

namespace dataproc {
    
    static struct { const char * xbottom; const char * xtop; const char * yleft; const char * yright; } 
        axis_labels [] = {
            { "flight length(m)"
              , 0
              , "time (&mu;s)"
              ,  0
            },
            { "&radic;<span style=\"text-decoration: overline\">&nbsp;<i>m/z</i></span>"
              , 0
              , "Slope(&mu;s/m)"
              , "Intercept(&mu;s)"
            }
        };
}

MSCalibSpectraWnd::MSCalibSpectraWnd( QWidget * parent ) : QWidget( parent )
                                                         , wndCalibSummary_( 0 )
                                                         , wndSplitter_( 0 )
                                                         , axis_( adwplot::SpectrumWidget::HorizontalAxisMass )
                                                         , regressionCurve_(0)
    , slopePlotCurve_(0)
    , interceptPlotCurve_(0)
{
    plots_.push_back( std::make_shared< adwplot::Dataplot >() ); // idPlotLengthTime
    plots_.push_back( std::make_shared< adwplot::Dataplot >() ); // idPlotSlopeIntercept

    for ( auto& plot: plots_ ) {
        plot->setMinimumHeight( 40 );
        plot->setMinimumWidth( 40 );
        QwtPlotGrid * grid = new QwtPlotGrid;
        grid->setMajorPen( Qt::gray, 0, Qt::DotLine );
        grid->attach( plot.get() );
    }

	QFont font;
	font.setFamily( "Verdana" );
	font.setBold( true );
	font.setItalic( true );
	font.setPointSize( 9 );

    int id = 0;
    for ( auto& plot: plots_ ) {
        auto& label = axis_labels[ id++ ];

        if ( label.xbottom ) {
            QwtText text( label.xbottom, QwtText::RichText );
            text.setFont( font );
            plot->setAxisTitle(QwtPlot::xBottom, text );
        }
        if ( label.yleft ) {
            QwtText text( label.yleft, QwtText::RichText );
            text.setFont( font );
            plot->setAxisTitle(QwtPlot::yLeft, text);
        }
        if ( label.yright ) {
            QwtText text( label.yright, QwtText::RichText );
            text.setFont( font );
            plot->setAxisTitle(QwtPlot::yRight, text);
            plot->enableAxis( QwtPlot::yRight );
        }
        plot->axisAutoScale( QwtPlot::xBottom );
        plot->axisAutoScale( QwtPlot::yLeft );
        plot->axisScaleEngine( QwtPlot::xBottom )->setAttribute( QwtScaleEngine::Floating, true );
        QwtPlotLegendItem * legendItem = new QwtPlotLegendItem;
        legendItem->attach( plot.get() );
        // legendItem->setMaxColumns( 1 );
    }
    init();
}

void
MSCalibSpectraWnd::init()
{
    Core::MiniSplitter * splitter = new Core::MiniSplitter;  // spectra | table
    if ( splitter ) {
        // spectrum on left
        wndSplitter_ = new Core::MiniSplitter;
        splitter->addWidget( wndSplitter_ );

        for ( int i = 0; i < 2; ++i ) {

            std::shared_ptr< adwplot::SpectrumWidget > wnd = std::make_shared< adwplot::SpectrumWidget >(this);
            wnd->setAutoAnnotation( false );
            wnd->axisWidget( QwtPlot::yLeft )->scaleDraw()->setMinimumExtent( 50 );
            wnd->setMinimumHeight( 40 );
            wndSpectra_.push_back( wnd );
            wndSplitter_->addWidget( wnd.get() );
            if ( i > 0 )
                wnd->link( wndSpectra_[ i - 1 ].get() );

            std::shared_ptr< QwtPlotMarker > marker = std::make_shared< QwtPlotMarker >();
            markers_.push_back( marker );
            marker->attach( wnd.get() );

        }
        wndSpectra_[ 0 ]->link( wndSpectra_.back().get() );

        wndSplitter_->setOrientation( Qt::Vertical );

        Core::MiniSplitter * splitter2 = new Core::MiniSplitter; // left pane split top (table) & bottom (time,mass plot)
        
        // summary table
		if ( ( wndCalibSummary_ = adplugin::widget_factory::create( L"qtwidgets2::MSCalibSummaryWidget" ) ) ) {
            bool res;
            res = connect( wndCalibSummary_, SIGNAL( currentChanged( size_t, size_t ) ), this, SLOT( handleSelSummary( size_t, size_t ) ) );
            assert( res );
            res = connect( wndCalibSummary_, SIGNAL( valueChanged() ), this, SLOT( handleValueChanged() ) );
            assert( res );
            res = connect( wndCalibSummary_, SIGNAL( on_recalibration_requested() ), this, SLOT( handle_recalibration_requested() ) );
            assert( res );
            res = connect( wndCalibSummary_, SIGNAL( on_reassign_mass_requested() ), this, SLOT( handle_reassign_mass_requested() ) );
            assert( res );
            res = connect( wndCalibSummary_, SIGNAL( on_apply_calibration_to_dataset()), this, SLOT( handle_apply_calibration_to_dataset() ) );
            assert(res);
            res = connect( wndCalibSummary_, SIGNAL( on_apply_calibration_to_default()), this, SLOT( handle_apply_calibration_to_default() ) );
            assert(res);

            res = connect( wndCalibSummary_, SIGNAL( on_add_selection_to_peak_table(const adcontrols::MSPeaks& ))
                           , this, SLOT( handle_add_selection_to_peak_table( const adcontrols::MSPeaks& ) ) );

            (void)res;

            adplugin::LifeCycleAccessor accessor( wndCalibSummary_ );
            adplugin::LifeCycle * p = accessor.get();
            if ( p )
                p->OnInitialUpdate();
            
            connect( this, SIGNAL( onSetData( const adcontrols::MSCalibrateResult&, const adcontrols::MassSpectrum& ) ),
                         wndCalibSummary_, SLOT( setData( const adcontrols::MSCalibrateResult&, const adcontrols::MassSpectrum& ) ) );
            
            splitter2->addWidget( wndCalibSummary_ );
        }
        if ( Core::MiniSplitter * splitter3 = new Core::MiniSplitter ) {// time vs length | slope, intercept vs m/z
            for ( auto& plot: plots_ )
                splitter3->addWidget( plot.get() );
            splitter3->setOrientation( Qt::Horizontal );
            splitter2->addWidget( splitter3 );
        }
        splitter2->setOrientation( Qt::Vertical );
        splitter->addWidget( splitter2 );

        splitter->setOrientation( Qt::Horizontal );
    }

    QBoxLayout * toolBarAddingLayout = new QVBoxLayout( this );

    toolBarAddingLayout->setMargin(0);
    toolBarAddingLayout->setSpacing(0);
    toolBarAddingLayout->addWidget( splitter );
}

void
MSCalibSpectraWnd::handleSessionAdded( Dataprocessor * processor )
{
    portfolio::Portfolio portfolio = processor->getPortfolio();

    if ( portfolio::Folder fCalib = portfolio.findFolder( L"MSCalibration" ) ) {

		fullpath_ = QString::fromStdWString( processor->filename() );

        if ( portfolio::Folium fSummary = fCalib.findFoliumByName( L"Summary Spectrum" ) ) {

			if ( fSummary.empty() )
				processor->fetch( fSummary );

			try {
				margedSpectrum_ = boost::any_cast< adcontrols::MassSpectrumPtr >( fSummary );
			} catch ( std::exception& ex ) {
				adportable::debug(__FILE__, __LINE__) << "got an exception: " << ex.what();
			}
            portfolio::Folio atts = fSummary.attachments();
            auto it = portfolio::Folium::find< adcontrols::MSCalibrateResultPtr >( atts.begin(), atts.end() );
            if ( it != atts.end() ) {
				try {
					margedCalibResult_ = boost::any_cast< adcontrols::MSCalibrateResultPtr >( *it );
				} catch ( std::exception& ex ) {
					adportable::debug(__FILE__, __LINE__) << "got an exception: " << ex.what();
				}
			}
        }
        if ( margedSpectrum_ ) {
            wndSpectra_[ 0 ] ->setData( margedSpectrum_, 0 );
            if ( margedCalibResult_ ) {
                emit onSetData( *margedCalibResult_, *margedSpectrum_ );
                flight_length_regression();
                plot_length_time();
            }
        }
    }
}

void
MSCalibSpectraWnd::handleAxisChanged( int axis )
{
    axis_ = axis;
    for ( auto wnd: wndSpectra_ )
		wnd->setAxis( static_cast< adwplot::SpectrumWidget::HorizontalAxis >( axis ) );
    
    replotSpectra();
}

void
MSCalibSpectraWnd::replotSpectra()
{
    if ( margedSpectrum_ ) {
        wndSpectra_[ 0 ]->setTitle( "Coadded spectrum" );
        wndSpectra_[ 0 ]->setData( margedSpectrum_, 0 );
    }

    size_t idx = 1;
    for ( auto& selected: selSpectra_ ) {

        if ( auto spectrum = selected.second.lock() ) {

            wndSpectra_[ idx ]->setData( spectrum, 0 );

            auto it = std::find_if( results_.begin(), results_.end(), [=]( const result_type& a ){
                    return selected.first == std::get<0>(a);
                });
            std::wstring title = ( it == results_.end() ) ? L"Unchecked: " : L"Checked: ";
			title += spectrum->getDescriptions().toString();
            wndSpectra_[ idx ]->setTitle( title );

            if ( ++idx >= wndSpectra_.size() )
                break;
        }
	}
}

void
MSCalibSpectraWnd::flight_length_regression()
{
    data_.clear();
    coeffs_intercepts_.clear();
    coeffs_slopes_.clear();

	if ( ! margedCalibResult_ )
		return;

    adcontrols::ChemicalFormula parser;

    const adcontrols::MSAssignedMasses& masses = margedCalibResult_->assignedMasses();
    adcontrols::segment_wrapper<> segments( *margedSpectrum_ );

    for ( auto& assigned: masses ) {
        std::wstring formula = parser.standardFormula( assigned.formula() ); // normalize 
		auto& ms = segments[ assigned.idMassSpectrum() ];
        
        auto& d = data_[ formula ];
        if ( !d ) 
            d = std::make_shared< internal::SeriesData >( formula, assigned.exactMass() );
        *d << internal::time_length( assigned.enable()
                                     , adcontrols::metric::scale_to_micro( assigned.time() )
                                     , ms.scanLaw().fLength( assigned.mode() ) );
    }
    //----------
    if ( data_.size() >= 2 ) {
        std::vector< double > mq, intercepts, slopes;
        
        for ( auto& d: data_ ) {
            double a, b;
			if ( d.second->polfit( a, b ) ) {
                intercepts.push_back( a );
                slopes.push_back( b );
                mq.push_back( std::sqrt( d.second->exactMass() ) );
            }
        }
        adportable::polfit::fit( slopes.data(), mq.data(), mq.size(), 2, coeffs_slopes_ ); // 90deg rotated against plot
        adportable::polfit::fit( mq.data(), intercepts.data(), mq.size(), 2, coeffs_intercepts_ );
    }
}

void
MSCalibSpectraWnd::plot_length_time()
{
    plotCurves_.clear();
    int idx = 0;
    for ( auto& d: data_ )
		plot_length_time( *d.second, idx++, *plots_[ idPlotLengthTime ] );

    if ( data_.size() > 0 ) {
        double sumT0 = 0;
        for ( auto& datum: data_ )
            sumT0 += datum.second->intercept();

        std::ostringstream o;
        o << boost::format( "<i>T<sub>0</sub></i> = %.7lf&mu;s" ) % (sumT0 / data_.size());
        plots_[ idPlotLengthTime ]->setTitle( o.str() );
    }

    plot_slope( *plots_[ idPlotSlopeIntercept ] );
    plot_intercept( *plots_[ idPlotSlopeIntercept ] );
}

void
MSCalibSpectraWnd::handleCheckStateChanged( Dataprocessor* processor, portfolio::Folium& folium, bool isChecked )
{
    (void)processor;    (void)isChecked;

	using adcontrols::MSCalibration;
    using adcontrols::metric::micro;

    portfolio::Folder folder = folium.getParentFolder();
	if ( ! ( folder && folder.name() == L"MSCalibration" ) )
		return;

    // collect checked spectra into results_ tuple
	portfolio::Folium fSummary = folder.findFoliumByName( L"Summary Spectrum" );
	if ( fSummary.nil() ) {
		fSummary = folder.addFolium( L"Summary Spectrum" );
        fSummary.assign( std::make_shared< adcontrols::MassSpectrum >(), adcontrols::MassSpectrum::dataClass() );

        portfolio::Folium fResult = fSummary.addAttachment( L"Calibrate Result" );
        fResult.assign( std::make_shared< adcontrols::MSCalibrateResult >(), adcontrols::MSCalibrateResult::dataClass() );

        SessionManager::instance()->updateDataprocessor( processor, fSummary );
    }
    
	auto fcalibResult = portfolio::find_first_of( fSummary.attachments(), []( const portfolio::Folium& f ){
			return f.dataClass() == adcontrols::MSCalibrateResult::dataClass();
		});
    if ( ! fcalibResult ) 
        return;

    margedSpectrum_ = portfolio::get< adcontrols::MassSpectrumPtr >( fSummary );
    margedCalibResult_ = portfolio::get< adcontrols::MSCalibrateResultPtr >( fcalibResult );
	
	coeffs_slopes_.clear();
	populate( processor, folder );
    generate_marged_result( processor );
	flight_length_regression();
	plot_length_time();

    if ( !coeffs_slopes_.empty() && !coeffs_intercepts_.empty() ) {
        MSCalibration calib( coeffs_slopes_, micro, coeffs_intercepts_, MSCalibration::MULTITURN_NORMALIZED );
        margedCalibResult_->calibration( calib );
    }

    if ( margedCalibResult_ && margedSpectrum_ )
        emit onSetData( *margedCalibResult_, *margedSpectrum_ );
    
    if ( margedSpectrum_ )
        wndSpectra_[ 0 ] ->setData( margedSpectrum_, 0 );
}

void
MSCalibSpectraWnd::handleSelectionChanged( Dataprocessor* processor, portfolio::Folium& folium )
{
    (void)processor;
    using adcontrols::MSCalibration;
    using adcontrols::metric::micro;

    portfolio::Folder folder = folium.getParentFolder();
	if ( ! ( folder && folder.name() == L"MSCalibration" ) )
		return;

    fullpath_ = QString::fromStdWString( processor->filename() );
    portfolio::Folio attachments = folium.attachments();

    portfolio::Folio::iterator it = std::find_if( attachments.begin(), attachments.end(), []( const portfolio::Folium& a ) {
            return a.dataClass() == adcontrols::MassSpectrum::dataClass() && a.attribute( L"name" ) == L"Centroid Spectrum";
        });
    if ( it == attachments.end() )
        return;

    std::weak_ptr< adcontrols::MassSpectrum > currSpectrum = boost::any_cast< adutils::MassSpectrumPtr >( *it );
    selSpectra_.push_front( std::make_pair( folium.id(), currSpectrum ) );
    if ( selSpectra_.size() > 3 )
        selSpectra_.pop_back();

    replotSpectra();
    flight_length_regression();
	plot_length_time();
}

void
MSCalibSpectraWnd::handleApplyMethod( const adcontrols::ProcessMethod& )
{
}

//
void 
MSCalibSpectraWnd::handleSelSummary( size_t idx, size_t fcn )
{
    using namespace adcontrols::metric;

    // size_t idSpectrum = fcn >> 16;
    adcontrols::MSAssignedMasses assigned;
    
    adcontrols::segment_wrapper<> segments( *margedSpectrum_ );
    double time = segments[ fcn ].getTime( idx );
    do {
        auto sp = segments[ fcn ];
        QwtPlotMarker& marker = *markers_[0];
        double x = ( axis_ == adwplot::SpectrumWidget::HorizontalAxisMass )
            ? sp.getMass( idx )
            : scale_to_micro( sp.getTime( idx ) );
        marker.setValue( x, sp.getIntensity( idx ) );
        marker.setLineStyle( QwtPlotMarker::Cross );
        marker.setLinePen( Qt::gray, 0.0, Qt::DashDotLine );
		wndSpectra_[0]->replot();
    } while(0);
        
    selFormula_.clear();
    if ( readCalibSummary( assigned ) ) {

        auto it = std::find_if( assigned.begin(), assigned.end(), [idx,fcn]( const adcontrols::MSAssignedMass& a ){
                return a.idPeak() == idx && a.idMassSpectrum() == fcn;
            });
        if ( it != assigned.end() ) {
            selFormula_ = it->formula();
            plotSelectedLengthTime( it->formula() );
        }
    }

    size_t nwnd = 1;
    for( auto& selected: selSpectra_ ) {
        if ( auto sp = selected.second.lock() ) {
            int idx = assign_peaks::find_by_time( *sp, time, 3.0e-9 );
            if ( idx >= 0 ) {
                QwtPlotMarker& marker = *markers_[nwnd];
                double x = ( axis_ == adwplot::SpectrumWidget::HorizontalAxisMass )
                    ? sp->getMass( idx )
                    : scale_to_micro( sp->getTime( idx ) );
                marker.setValue( x, sp->getIntensity( idx ) );
                marker.setLineStyle( QwtPlotMarker::Cross );
                marker.setLinePen( Qt::gray, 0.0, Qt::DashDotLine );
            }
            if ( ++nwnd >= wndSpectra_.size() )
                break;
        }
    }
}

void
MSCalibSpectraWnd::handleValueChanged()
{
    adcontrols::MSAssignedMasses assigned;

    if ( readCalibSummary( assigned ) ) {
		margedCalibResult_->assignedMasses( assigned );
        if ( DataprocHandler::doAnnotateAssignedPeaks( *margedSpectrum_, assigned ) )
            wndSpectra_[ 0 ]->setData( margedSpectrum_, 0 ); // replot
    }

    flight_length_regression();
    plot_length_time();
	if ( !coeffs_slopes_.empty() ) {
		adcontrols::MSCalibration calib( coeffs_slopes_, adcontrols::metric::micro, coeffs_intercepts_, adcontrols::MSCalibration::MULTITURN_NORMALIZED );
		margedCalibResult_->calibration( calib );        
	}

    if ( ! selFormula_.empty() )
        plotSelectedLengthTime( selFormula_ );
    else if ( data_.begin() != data_.end() )
        plotSelectedLengthTime( data_.begin()->first );

	emit onSetData( *margedCalibResult_, *margedSpectrum_ );
}

int
MSCalibSpectraWnd::populate( Dataprocessor * processor, portfolio::Folder& folder )
{
    results_.clear();
    
    portfolio::Folio folio = folder.folio();

    std::for_each( folio.begin(), folio.end(), [&]( portfolio::Folium& item ){

            if ( item.name() != L"Summary Spectrum" && item.attribute( L"isChecked" ) == L"true" ) {
                
                if ( item.empty() )
                    processor->fetch( item );

                adcontrols::MassSpectrumPtr ms;
                adcontrols::MSCalibrateResultPtr calib;

                portfolio::Folio attachments = item.attachments();

				if ( auto fCentroid = portfolio::find_first_of( attachments, []( const portfolio::Folium& a ){
                            return a.name() == Constants::F_CENTROID_SPECTRUM; } ) ) {
                    ms = portfolio::get< adcontrols::MassSpectrumPtr >( fCentroid );
				}

                if ( auto fcalibResult = portfolio::find_first_of( attachments, []( portfolio::Folium& a ){
                            return portfolio::is_type<adcontrols::MSCalibrateResultPtr>( a ); } ) ) {
                    calib = portfolio::get< adcontrols::MSCalibrateResultPtr >( fcalibResult );
                }

                auto result = std::make_tuple( item.id(), ms, calib );
                results_.push_back( result );
            }
        });

    return static_cast< int >( results_.size() );
}

bool
MSCalibSpectraWnd::readCalibSummary( adcontrols::MSAssignedMasses& assigned )
{
    adplugin::LifeCycleAccessor accessor( wndCalibSummary_ );
    adplugin::LifeCycle * p = accessor.get();
    if ( p ) {
        std::shared_ptr< adcontrols::MSAssignedMasses > ptr( new adcontrols::MSAssignedMasses );
        boost::any any( ptr );
        if ( p->getContents( any ) ) {
            assigned = *ptr;
            return true;
        }
    }
    return false;
}

void
MSCalibSpectraWnd::handle_reassign_mass_requested()
{
    using namespace adcontrols::metric;

    adcontrols::MSAssignedMasses assigned;
    if ( margedCalibResult_ ) {

        assigned = margedCalibResult_->assignedMasses();
        const adcontrols::MSCalibration& calib = margedCalibResult_->calibration();
        adcontrols::segment_wrapper<> segments( *margedSpectrum_ );

        if ( calib.algorithm() == adcontrols::MSCalibration::MULTITURN_NORMALIZED ) {
			adcontrols::ComputeMass< adcontrols::ScanLaw > mass_calculator( margedSpectrum_->scanLaw(), calib );
            for ( auto& a: assigned ) {
				double mass = mass_calculator( a.time(), a.mode() );
				a.mass( mass );
                segments[ a.idMassSpectrum() ].setMass( a.idPeak(), mass );
            }
        }
		DataprocHandler::doAnnotateAssignedPeaks( *margedSpectrum_, assigned );
		margedCalibResult_->assignedMasses( assigned );
        emit onSetData( *margedCalibResult_, *margedSpectrum_ );
    }
    
}

void
MSCalibSpectraWnd::handle_recalibration_requested()
{
    QMessageBox::information( 0, "MSCalibSpectra", "recalibration not implementd" );
}

void
MSCalibSpectraWnd::handle_apply_calibration_to_dataset()
{
    QMessageBox::information( 0, "MSCalibSpectra", "apply calibration to dataset not implementd" );
}

void
MSCalibSpectraWnd::handle_add_selection_to_peak_table( const adcontrols::MSPeaks& peaks )
{
    MainWindow::instance()->handle_add_mspeaks( peaks );
}

void
MSCalibSpectraWnd::handle_apply_calibration_to_default()
{
	if ( margedCalibResult_ && margedSpectrum_ ) {
		handle_reassign_mass_requested();
		Dataprocessor::saveMSCalibration( *margedCalibResult_, *margedSpectrum_ );
	}
}

void
MSCalibSpectraWnd::handlePrintCurrentView( const QString& pdfname )
{
	// A4 := 210mm x 297mm (8.27 x 11.69 inch)
    QPrinter printer;
    printer.setColorMode( QPrinter::Color );
    printer.setPaperSize( QPrinter::A4 );
    printer.setFullPage( false );
	printer.setOrientation( QPrinter::Landscape );
    
    printer.setDocName( "QtPlatz Multi-turn calibration report" );
    printer.setOutputFileName( pdfname );
    // printer.setResolution( resolution );

    QPainter painter( &printer );

    QRectF boundingRect;
    QRectF drawRect( printer.resolution()/2, printer.resolution()/2, printer.width() - printer.resolution(), (12.0/72)*printer.resolution() );
	painter.drawText( drawRect, Qt::TextWordWrap, fullpath_, &boundingRect );

    drawRect.setTop( boundingRect.bottom() );
    drawRect.setHeight( printer.height() - boundingRect.top() - printer.resolution()/2 );
    //drawRect.setWidth( size.width() );

    QwtPlotRenderer renderer;
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasBackground, true );
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasFrame, true );
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardBackground, true );

    QRectF rc( drawRect );
	rc.setHeight( drawRect.height() / 2 );
    rc.setWidth( drawRect.width() * 0.6 );
    for ( auto& plot: plots_ ) {
        renderer.render( plot.get(), &painter, rc );
        rc.moveTo( rc.left(), rc.bottom() );
    }

    // renderer.render( pImpl_->processedSpectrum_, &painter, drawRect );
    // ---------- calibratin equation ----------
#if 0
    if ( calibResult ) {
        const adcontrols::MSCalibration& calib = calibResult->calibration();
        QString text = "Calibration eq.: sqrt(m/z) = ";
        for ( size_t i = 0; i < calib.coeffs().size(); ++i ) {
            if ( i == 0 )
                text += ( boost::format( "%.14le" ) % calib.coeffs()[0] ).str().c_str();
            else
                text += ( boost::format( "\t+ %.14le * x^%d" ) % calib.coeffs()[i] % i ).str().c_str();
        }
        drawRect.setTop( drawRect.bottom() + 0.5 * resolution );
        drawRect.setHeight( printer.height() - drawRect.top() );
        QFont font = painter.font();
        font.setPointSize( 8 );
        painter.setFont( font );
        painter.drawText( drawRect, Qt::TextWordWrap, text );
    }
#endif
    // ---------- end calibration equestion -----

    if ( connect( this, SIGNAL( onPrint(QPrinter&, QPainter&) )
                  , wndCalibSummary_, SLOT(handlePrint(QPrinter&, QPainter&)) ) ) {
        emit onPrint( printer, painter );
        bool res = disconnect( this, SIGNAL( onPrint(QPrinter&, QPainter&) )
                               , wndCalibSummary_, SLOT(handlePrint(QPrinter&, QPainter&)) );
        assert( res );
    }
    
}


void
MSCalibSpectraWnd::plot_length_time( internal::SeriesData& d, int id, adwplot::Dataplot& plot )
{
    static Qt::GlobalColor colors [] = {
        Qt::blue, Qt::red, Qt::green, Qt::cyan, Qt::magenta, Qt::yellow
        , Qt::darkRed, Qt::darkGreen, Qt::darkBlue, Qt::darkCyan, Qt::darkMagenta
    };

	plotCurves_[ d.formula() ] = std::make_shared< QwtPlotCurve >();
	auto plotCurve = plotCurves_[ d.formula() ];

	plotCurve->attach( &plot );
    plotCurve->setLegendAttribute( QwtPlotCurve::LegendShowLine );
	plotCurve->setSymbol( new QwtSymbol( QwtSymbol::Style( QwtSymbol::Ellipse + (id % 10) ), Qt::NoBrush, QPen( Qt::darkMagenta ), QSize(5, 5 ) ) );
    plotCurve->setPen( QPen( colors[ id % 11 ] ) );
    plotCurve->setTitle( QString::fromStdWString( adcontrols::ChemicalFormula::formatFormula( d.formula() ) ) );
    plotCurve->setStyle( QwtPlotCurve::NoCurve );

    std::shared_ptr< internal::SeriesData > ptr = d.shared_from_this();
    plotCurve->setData( new internal::xSeriesData( ptr ) );
	plot.axisAutoScale( QwtPlot::xBottom );
	plot.axisAutoScale( QwtPlot::yLeft );

    plotRegressions_[ d.formula() ] = std::make_shared< QwtPlotCurve >();
    double a, b;
    if ( d.polfit( a, b ) ) {
        QVector< QPointF > xy;
        xy << QPointF( -(a/b), 0.0 );
        const QRectF& rc = d.boundingRect();
        const double x = rc.right() + rc.width() * 0.1; // extend 10% of width
        xy << QPointF( x, adportable::polfit::estimate_y( d.coeffs(), x ) );
        QwtPlotCurve& curve = *plotRegressions_[ d.formula() ];
        curve.setSamples( xy );
		curve.setTitle( QString::fromStdWString( adcontrols::ChemicalFormula::formatFormula( d.formula() ) + L" regression") );
        curve.setPen( QPen( colors[ id % 11 ] ) );
        curve.attach( &plot );
    }
	plot.replot();
	plot.zoomer().setZoomBase( false );
}

void
MSCalibSpectraWnd::plot_slope( adwplot::Dataplot& plot )
{
	plot.axisAutoScale( QwtPlot::xBottom );
	plot.axisAutoScale( QwtPlot::yLeft );
	plot.axisAutoScale( QwtPlot::yRight );

    slopeMarkers_.clear();
    QRectF rect;

    int n = 0;
    for ( auto& datum: data_ ) {
		QString formula = QString::fromStdWString( datum.first );
        std::shared_ptr< QwtPlotMarker > marker = std::make_shared< QwtPlotMarker >( formula );
        slopeMarkers_.push_back( marker );
		marker->setLabel( QwtText( QString::fromStdWString( adcontrols::ChemicalFormula::formatFormula( datum.first ).c_str() ) ) );
		marker->setLabelAlignment( Qt::AlignRight | Qt::AlignBottom );
		marker->setSymbol( new QwtSymbol( QwtSymbol::Style( QwtSymbol::Cross ), Qt::NoBrush, QPen( Qt::blue ), QSize(5, 5) ) );
        double x = std::sqrt( datum.second->exactMass() );
        double y = datum.second->slope();
        marker->setValue( x, y );
        if ( n++ == 0 )
            rect = QRectF( x, y, 0, 0 );
        else
            rect.setCoords( std::min( x, rect.left() ), std::min( y, rect.top() ), std::max( x, rect.right() ), std::max( x, rect.bottom() ) );
        marker->attach( &plot );
    }

    slopePlotCurve_ = std::make_shared< QwtPlotCurve >(); // clear if plot exists
    slopePlotCurve_->attach( &plot );
	slopePlotCurve_->setLegendAttribute( QwtPlotCurve::LegendShowLine );
    slopePlotCurve_->setPen( QPen( Qt::blue ) );
    slopePlotCurve_->setYAxis( QwtPlot::yLeft );
    slopePlotCurve_->setTitle( "slope" );

    if ( coeffs_slopes_.size() >= 2 ) {
        double x1 = rect.left() - rect.width() * 0.05;  // sqrt(m) axis  := horizontal axis but y on polymonials
        double x2 = rect.right() + rect.width() * 0.05; // sqrt(m) axis
        double y1 = ( x1 - coeffs_slopes_[ 0 ] ) / coeffs_slopes_[ 1 ];
        double y2 = ( x2 - coeffs_slopes_[ 0 ] ) / coeffs_slopes_[ 1 ];
        QVector< QPointF > xy;
        xy << QPointF( x1, y1 );
        xy << QPointF( x2, y2 );
        slopePlotCurve_->setSamples( xy );
    }
    do {
        std::ostringstream o;
        adcontrols::MSCalibration calib;
        calib.coeffs( coeffs_slopes_ );
        o << "Slope: " << calib.formulaText() << ";  ";
        plot.setTitle( o.str() );
    } while(0);
    plot.replot();
    plot.zoomer().setZoomBase( false );
}

void
MSCalibSpectraWnd::plot_intercept( adwplot::Dataplot& plot )
{
    QRectF rect;

    int n = 0;
    for ( auto& datum: data_ ) {
		QString formula = QString::fromStdWString( datum.first );
        std::shared_ptr< QwtPlotMarker > marker = std::make_shared< QwtPlotMarker >( formula );
        slopeMarkers_.push_back( marker );
		marker->setLabel( QwtText( QString::fromStdWString( adcontrols::ChemicalFormula::formatFormula( datum.first ).c_str() ) ) );
		marker->setLabelAlignment( Qt::AlignLeft | Qt::AlignTop );
		marker->setSymbol( new QwtSymbol( QwtSymbol::Style( QwtSymbol::Cross ), Qt::NoBrush, QPen( Qt::red ), QSize(5, 5) ) );
        marker->setYAxis( QwtPlot::yRight );
        double x = std::sqrt( datum.second->exactMass() );
        double y = datum.second->intercept();
        marker->setValue( x, y );
        if ( n++ == 0 )
            rect = QRectF( x, y, 0, 0 );
        else
            rect.setCoords( std::min( x, rect.left() ), std::min( y, rect.top() ), std::max( x, rect.right() ), std::max( x, rect.bottom() ) );
        marker->attach( &plot );
    }

    interceptPlotCurve_ = std::make_shared<QwtPlotCurve>();
    interceptPlotCurve_->attach( &plot );
    interceptPlotCurve_->setLegendAttribute( QwtPlotCurve::LegendShowLine );
    interceptPlotCurve_->setPen( QPen( Qt::red ) );
    interceptPlotCurve_->setYAxis( QwtPlot::yRight );
    interceptPlotCurve_->setTitle( "intercept" );
    
    if ( coeffs_intercepts_.size() >= 2 ) {
        double x1 = rect.left() - rect.width() * 0.05;  // sqrt(m) axis  := horizontal axis but y on polymonials
        double x2 = rect.right() + rect.width() * 0.05; // sqrt(m) axis
        double y1 = adportable::polfit::estimate_y( coeffs_intercepts_, x1 );
        double y2 = adportable::polfit::estimate_y( coeffs_intercepts_, x2 );
        QVector< QPointF > xy;
        xy << QPointF( x1, y1 );
        xy << QPointF( x2, y2 );
        interceptPlotCurve_->setSamples( xy );
    }

	if ( coeffs_intercepts_.size() >= 1 ) {
        std::ostringstream o;
        o << boost::format( "intercept = %.6g" ) % coeffs_intercepts_[0];
        for ( size_t i = 1; i < coeffs_intercepts_.size(); ++i ) {
            o << boost::format(
                " + %.6g &times; &radic;<span style=\"text-decoration: overline\">&nbsp;<i>m/z</i></span><sup>%d</sup>" )
                % coeffs_intercepts_[ i ] % i;
        }
        o << "; ";
        plot.setFooter( o.str() );
    } while(0);

    plot.replot();
    plot.zoomer().setZoomBase( false );
}

void
MSCalibSpectraWnd::plotSelectedLengthTime( const std::wstring& formula )
{
    using namespace adcontrols::metric;

    auto& dplot = plots_[ idPlotLengthTime ];

	adcontrols::ChemicalFormula parser;
	std::wstring stdformula = parser.standardFormula( formula );
    auto it = data_.find( stdformula );
	if ( it != data_.end() ) {
        // SeriesData has microseconds scale
        internal::SeriesData& data = *it->second;
        double a = 0, b = 0;
        if ( data.polfit( a, b ) ) {
            double Lo = -(a / b);

            std::ostringstream o;
            o << boost::format( "%s: <i>t(&mu;s) = %.14le + %.14le &times; L</i>, L<sub>0</sub>=%.8lfm" )
                % adportable::utf::to_utf8( adcontrols::ChemicalFormula::formatFormula( formula ) )
                % a
                % b
                % Lo;

            dplot->setFooter( o.str().c_str() );
            if ( regressionCurve_ == 0 ) {
                regressionCurve_ = new QwtPlotCurve;
                regressionCurve_->attach( dplot.get() );
            }
            
            const QRectF& rc = dplot->zoomRect();
            // const QRectF& rc = it->second->boundingRect();
            double Lx = rc.right() + rc.width() / 20.0;
            double Tx = a + b * Lx;
            QVector< QPointF > xy;
            xy << QPointF( Lo, 0 );
            xy << QPointF( Lx, Tx );
            regressionCurve_->setSamples( xy );
            regressionCurve_->setTitle( QString::fromStdWString( adcontrols::ChemicalFormula::formatFormula( formula ) + L" regression") );
            regressionCurve_->setPen( Qt::gray, 2.0, Qt::DashDotLine );
        } else {
            delete regressionCurve_;
            regressionCurve_ = 0;
            std::ostringstream o;
            o << adportable::utf::to_utf8( adcontrols::ChemicalFormula::formatFormula( formula ) )
              << " linear regression failed";
            dplot->setFooter( o.str() );
        }
        dplot->replot();
    }
}

void
MSCalibSpectraWnd::generate_marged_result( Dataprocessor * /* processor */)
{
    if ( ! ( margedSpectrum_ && margedCalibResult_ ) )
        return;

    margedSpectrum_->clearSegments();
    margedSpectrum_->resize(0);

    adcontrols::MSAssignedMasses masses;

    size_t idSpectrum = 0;

    for ( auto result: results_ ) {

        auto ms = std::get<1>( result ).lock();
        auto calibResult = std::get<2>( result ).lock();
        if ( ! ( ms && calibResult ) )
            continue;
        if ( ! ms->isCentroid() )
            continue;

        adcontrols::segment_wrapper<> segments( *ms );

        adcontrols::MassSpectrum clone;
        clone.clone( segments[0] );
        clone.resize( calibResult->assignedMasses().size() );

        // reconstract mass spectrum and corresponding MSAssignedMasses
        int idPeak = 0;
        for ( auto& t: calibResult->assignedMasses() ) {

			auto& fms = segments[ t.idMassSpectrum() ];
            clone.setMass( idPeak, fms.getMass( t.idPeak() ) );
            clone.setIntensity( idPeak, fms.getIntensity( t.idPeak() ) );
            clone.setTime( idPeak, fms.getTime( t.idPeak() ) );
			if ( const unsigned char * colors = fms.getColorArray() )
                clone.setColor( idPeak, colors[ t.idPeak() ] );

            adcontrols::MSAssignedMass x( t );
            x.idMassSpectrum( static_cast<uint32_t>(idSpectrum) );
            x.idPeak( idPeak );
            masses << x;
            ++idPeak;
        }
        if ( idSpectrum == 0 )
            *margedSpectrum_ = clone;
        else
            margedSpectrum_->addSegment( clone );
        ++idSpectrum; // move on to next spectrum
    }
    margedSpectrum_->addDescription( adcontrols::Description( L"Calibration", L"Coadded Summary" ) );
	margedCalibResult_->assignedMasses( masses );
    if ( masses.size() > 0 )
        DataprocHandler::doAnnotateAssignedPeaks( *margedSpectrum_, masses );
}


