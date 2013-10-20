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
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/msassignedmass.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/mscalibration.hpp>
#include <adcontrols/mscalibrateresult.hpp>
#include <adcontrols/mscalibratemethod.hpp>
#include <adcontrols/chemicalformula.hpp>
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
#include <qwt_scale_widget.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_legenditem.h>
#include <qwt_symbol.h>
#include <qwt_scale_engine.h>
#include <qwt_legend.h>
#include <QVBoxLayout>
#include <boost/format.hpp>
#include <cmath>
#include <tuple>

namespace dataproc {
    namespace internal {
		class SeriesData : public std::enable_shared_from_this< SeriesData >
                         , boost::noncopyable {
            std::vector< std::tuple< bool, double, double > > d_; // enable, time, length
            std::wstring formula_;
            double exactMass_;
            QRectF rect_;
            double intercept_;
            double slope_;
        public:
            virtual ~SeriesData() {}
            SeriesData( const std::wstring& formula, double exactMass ) : formula_( formula )
                                                                        , exactMass_( exactMass )
                                                                        , intercept_(0)
                                                                        , slope_(0) {
            }
            const std::wstring& formula() const { return formula_; }
            double slope() const { return slope_; }
            double intercept() const { return intercept_; }
            double exactMass() const { return exactMass_; }

            size_t size() { return d_.size(); }
            QPointF sample( size_t idx ) const {
				return QPointF( std::get<2>(d_[ idx ]), std::get<1>(d_[ idx ]) ); // (length, time)
			}
            QRectF boundingRect() const {
				return rect_;
			}  

            // local member
            void clear() {  d_.clear(); }
            SeriesData& operator << ( const std::tuple< bool, double, double >& d ) {
                d_.push_back( d );
                rect_.setLeft( std::min( std::get<2>(d),   rect_.left() ) );
                rect_.setRight( std::max( std::get<2>(d),  rect_.right() ) );
                rect_.setTop( std::min( std::get<1>(d),    rect_.top() ) );
                rect_.setBottom( std::max( std::get<1>(d), rect_.bottom() ) );
                return *this;
            }
            
            bool polfit( double& a, double& b ) {
                std::vector< double > time, length, polinomial;
                for ( auto t: d_ ){
                    if ( std::get<0>(t) ) {
                        time.push_back( std::get<1>(t) ); // time
                        length.push_back( std::get<2>(t) ); // length
                    }
                }
                if ( adportable::polfit::fit( length.data(), time.data(), time.size(), 2, polinomial ) ) {
                    a = polinomial[0];
                    b = polinomial[1];
                    intercept_ = a;
                    slope_ = b;
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
	time_length_marker_->detach();
}

MSCalibSpectraWnd::MSCalibSpectraWnd( QWidget * parent ) : QWidget( parent )
                                                         , wndCalibSummary_( 0 )
                                                         , wndSplitter_( 0 )
                                                         , dplot_( new adwplot::Dataplot )
                                                         , rplot_( new adwplot::Dataplot )
                                                         , axis_( adwplot::SpectrumWidget::HorizontalAxisMass )
                                                         , regressionCurve_(0)
    , slopePlotCurve_(0)
    , interceptPlotCurve_(0)
    , slopeRegressionCurve_(0)
    , interceptRegressionCurve_(0)
    , T0_(0)
{
    dplot_->setMinimumHeight( 40 );
    dplot_->setMinimumWidth( 40 );
    rplot_->setMinimumHeight( 40 );
    rplot_->setMinimumWidth( 40 );

	QwtText text_haxis( "flight length(m)", QwtText::RichText );
	QwtText text_vaxis( "time (&mu;s)", QwtText::RichText );

	QFont font = text_haxis.font();
	font.setFamily( "Verdana" );
	font.setBold( true );
	font.setItalic( true );
	font.setPointSize( 9 );
	text_haxis.setFont( font );
    text_vaxis.setFont( font );

    dplot_->setAxisTitle(QwtPlot::xBottom, text_haxis);
    dplot_->setAxisTitle(QwtPlot::yLeft, text_vaxis);
	dplot_->axisAutoScale( QwtPlot::xBottom );
	dplot_->axisAutoScale( QwtPlot::yLeft );
	dplot_->axisScaleEngine( QwtPlot::xBottom )->setAttribute( QwtScaleEngine::Floating, true );
    do {
        QwtPlotLegendItem * legendItem = new QwtPlotLegendItem;
        legendItem->attach( dplot_.get() );
    } while(0);

    dplot_->setAxisTitle( QwtPlot::xBottom, text_haxis );
	dplot_->setAxisScale( QwtPlot::yLeft, -1.00, 50 );
	dplot_->setAxisScale( QwtPlot::xBottom, -1.0, 10.0 );
	time_length_marker_ = std::make_shared< QwtPlotMarker >();
    time_length_marker_->attach( dplot_.get() );
	//dplot_->zoomer().autoYScale( true );

    QwtText haxis( "&radic;<span style=\"text-decoration: overline\">&nbsp;<i>m/z</i></span>" );
    haxis.setFont( font );
    rplot_->setAxisTitle( QwtPlot::xBottom, haxis );

    QwtText left_axis( "Slope" );
    QwtText right_axis( "Intercept" );
    left_axis.setFont( font );
    right_axis.setFont( font );
    rplot_->setAxisTitle( QwtPlot::yLeft, left_axis );
	rplot_->enableAxis( QwtPlot::yRight );
    rplot_->setAxisTitle( QwtPlot::yRight, right_axis );
    do {
        QwtPlotLegendItem * legendItem = new QwtPlotLegendItem;
        legendItem->setMaxColumns( 1 );
        legendItem->attach( rplot_.get() );
    } while(0);

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

        for ( int i = 0; i < 3; ++i ) {

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
            splitter3->addWidget( dplot_.get() );
            splitter3->addWidget( rplot_.get() );
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
MSCalibSpectraWnd::handleSessionAdded( Dataprocessor * )
{
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
        wndSpectra_[ 0 ]->setTitle( "Spectra summay" );
        wndSpectra_[ 0 ]->setData( margedSpectrum_, 0 );
    }

    size_t idx = 1;
    for ( auto& selected: selSpectra_ ) {
        if ( auto spectrum = selected.second.lock() ) {
            wndSpectra_[ idx ]->setData( spectrum, 0 );
        }
        auto it = std::find_if( results_.begin(), results_.end(), [=]( const result_type& a ){
                return selected.first == std::get<0>(a);
            });
        if ( it != results_.end() )
            wndSpectra_[ idx ]->setTitle( "checked" );
        else
            wndSpectra_[ idx ]->setTitle( "unchecked" );

        if ( ++idx >= wndSpectra_.size() )
            break;
	}
}

void
MSCalibSpectraWnd::flight_length_regression()
{
    aCoeffs_.clear();
    bCoeffs_.clear();
    tofCoeffs_.clear();
    adcontrols::ChemicalFormula chemicalFormula;

    double t = 0;
    size_t n = 0;
    for ( auto& d: plotData_ ) {
        double a, b;
		if ( d.second->polfit( a, b ) ) {
            t += a;
            ++n;
            tofCoeffs_[ d.first ] = std::make_pair( a, b );
        }
	}
    T0_ = adcontrols::metric::scale_to_base( t / n, adcontrols::metric::micro );

    std::vector<double> mq, a, b;
    for ( auto d: tofCoeffs_ ) {
        double mass = chemicalFormula.getMonoIsotopicMass( d.first );
        mq.push_back( std::sqrt( mass ) );
        a.push_back( d.second.first ); // a, microsecond scale
        b.push_back( d.second.second ); // b, microsecond scale
    }

    std::ostringstream title;
    std::ostringstream footer;

    if ( mq.size() >= 2 ) {

        if ( adportable::polfit::fit( b.data(), mq.data(), mq.size(), 2, bCoeffs_ ) ) {
            adcontrols::MSCalibration calib;
            calib.coeffs( bCoeffs_ );
			title << "Slope: " << calib.formulaText() << ";  ";
        }

        if ( adportable::polfit::fit( mq.data(), a.data(), mq.size(), 2, aCoeffs_ ) ) {
            footer << boost::format( "intercept = %.6le" ) % aCoeffs_[0];
            for ( size_t i = 1; i < aCoeffs_.size(); ++i )
                footer << boost::format(
                    " + %.6le &times; &radic;<span style=\"text-decoration: overline\">&nbsp;<i>m/z</i></span><sup>%d</sup>" )
                    % aCoeffs_[ i ] % i;
			footer << ";  ";
        }
    }
    std::ostringstream o;
    o << boost::format( "T<sub>0</sub> = %.6lf&mu;s" ) % adcontrols::metric::scale_to_micro( T0_ );
    
	dplot_->setTitle( o.str() );
	rplot_->setTitle( title.str() );
	rplot_->setFooter( footer.str() );

    plot_slope();
    plot_intercept();
    rplot_->replot();
}

void
MSCalibSpectraWnd::replotLengthTime()
{
    plotCurves_.clear();
    plotData_.clear();

    adcontrols::ChemicalFormula parser;

    const adcontrols::MSAssignedMasses& masses = margedCalibResult_->assignedMasses();
    adcontrols::segment_wrapper<> segments( *margedSpectrum_ );

    for ( auto& assigned: masses ) {
        std::wstring formula = parser.standardFormula( assigned.formula() ); // normalize 
		auto& ms = segments[ assigned.idMassSpectrum() ];

        auto& d = plotData_[ formula ];
        if ( ! d ) {
            d = std::make_shared< internal::SeriesData >( formula, assigned.exactMass() );
        }
        *d << std::make_tuple( assigned.enable(), adcontrols::metric::scale_to_micro( assigned.time() ), ms.scanLaw().fLength( assigned.mode() ) );
    }

    int idx = 0;
    for ( auto& d: plotData_ )
		plot( *d.second, idx++ );

}

void
MSCalibSpectraWnd::handleCheckStateChanged( Dataprocessor* processor, portfolio::Folium& folium, bool isChecked )
{
    portfolio::Folder folder = folium.getParentFolder();
	if ( ! ( folder && folder.name() == L"MSCalibration" ) )
		return;

    (void)isChecked;
    (void)processor;
}

void
MSCalibSpectraWnd::handleSelectionChanged( Dataprocessor* processor, portfolio::Folium& folium )
{
    using adcontrols::MSCalibration;
    using adcontrols::metric::micro;

    portfolio::Folder folder = folium.getParentFolder();
	if ( ! ( folder && folder.name() == L"MSCalibration" ) )
		return;

    folio_ = folder.folio();

    folium_ = folium;
    portfolio::Folio attachments = folium.attachments();

    portfolio::Folio::iterator it
        = portfolio::Folium::find<adcontrols::MassSpectrumPtr>(attachments.begin(), attachments.end());
    if ( it == attachments.end() )
        return;

    std::weak_ptr< adcontrols::MassSpectrum > currSpectrum = boost::any_cast< adutils::MassSpectrumPtr >( *it );

    populate( processor, folder );
    generate_marged_result( processor );

    selSpectra_.push_front( std::make_pair( folium.id(), currSpectrum ) );
    if ( selSpectra_.size() > 3 )
        selSpectra_.pop_back();
    
    replotSpectra();
	replotLengthTime();
    flight_length_regression();
    
    MSCalibration calib( bCoeffs_, micro, aCoeffs_, MSCalibration::MULTITURN_NORMALIZED );
    margedCalibResult_->t0( T0_ );
    margedCalibResult_->calibration( calib );        
    
    emit onSetData( *margedCalibResult_, *margedSpectrum_ );

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

    replotLengthTime();
    flight_length_regression();
    margedCalibResult_->t0( T0_ );
	adcontrols::MSCalibration calib( bCoeffs_, adcontrols::metric::micro, aCoeffs_, adcontrols::MSCalibration::MULTITURN_NORMALIZED );
    margedCalibResult_->calibration( calib );        

    if ( ! selFormula_.empty() )
        plotSelectedLengthTime( selFormula_ );
    else if ( plotData_.begin() != plotData_.end() )
        plotSelectedLengthTime( plotData_.begin()->first );

    reverse_assign_peaks();

	emit onSetData( *margedCalibResult_, *margedSpectrum_ );
}

void
MSCalibSpectraWnd::reverse_assign_peaks()
{
    // update for original spectrum
    const adcontrols::MSAssignedMasses& assigned = margedCalibResult_->assignedMasses();

    for ( auto& result: results_ ) {

        adcontrols::MSAssignedMasses masses;

        for ( const adcontrols::MSAssignedMass& a: assigned ) {

            if ( adcontrols::MassSpectrumPtr ptr = std::get<1>( result ) ) {

                adcontrols::segment_wrapper<> segments( *ptr );
                int idSegment = 0;

                for ( const adcontrols::MassSpectrum& ms: segments ) {
                    const double * times = ms.getTimeArray();
                    auto it = std::lower_bound( times, times + ms.size(), a.time() );
                    if ( it != times + ms.size() && adportable::compare<double>::essentiallyEqual( *it, a.time() ) ) {
						adcontrols::MSAssignedMass x( a );
                        x.idPeak( std::distance( times, it ) );
                        x.idMassSpectrum( idSegment );
                        x.time( *it );  // just make sure
                        masses << x;
                    }
                }
                idSegment++;
            }
        }
        if ( auto calibResult = std::get<2>( result ) ) {
            calibResult->assignedMasses( masses );
        }
    }
}

int
MSCalibSpectraWnd::populate( Dataprocessor * processor, portfolio::Folder& folder )
{
    results_.clear();
    
    portfolio::Folio folio = folder.folio();

    std::for_each( folio.begin(), folio.end(), [&]( portfolio::Folium& item ){

            if ( item.attribute( L"isChecked" ) == L"true" ) {
                
                if ( item.empty() )
                    processor->fetch( item );

                adcontrols::MassSpectrumPtr ms;
                adcontrols::MSCalibrateResultPtr calib;

                portfolio::Folio attachments = item.attachments();
                auto a1 = portfolio::Folium::find< adcontrols::MassSpectrumPtr >( attachments.begin(), attachments.end() );
                if ( a1 != attachments.end() )
                    ms = boost::any_cast< adcontrols::MassSpectrumPtr >( *a1 );

                auto a2 = portfolio::Folium::find< adcontrols::MSCalibrateResultPtr >( attachments.begin(), attachments.end() );
                if ( a2 != attachments.end() )
                    calib = boost::any_cast< adcontrols::MSCalibrateResultPtr >( *a2 );

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
    adcontrols::MSAssignedMasses assigned;
    if ( readCalibSummary( assigned ) ) {
        //assert(0);
    }
}

void
MSCalibSpectraWnd::handle_recalibration_requested()
{
    adcontrols::MSAssignedMasses assigned;
    if ( readCalibSummary( assigned ) )
        MainWindow::instance()->applyCalibration( assigned, folium_ );
}

void
MSCalibSpectraWnd::handle_apply_calibration_to_dataset()
{
    assert(0);
}

void
MSCalibSpectraWnd::handle_apply_calibration_to_default()
{
    assert(0);
}

void
MSCalibSpectraWnd::plot( internal::SeriesData& d, int id )
{
    static Qt::GlobalColor colors [] = {
        Qt::blue, Qt::red, Qt::green, Qt::cyan, Qt::magenta, Qt::yellow
        , Qt::darkRed, Qt::darkGreen, Qt::darkBlue, Qt::darkCyan, Qt::darkMagenta
    };
    
	plotCurves_[ d.formula() ] = std::make_shared< QwtPlotCurve >();
	auto plotCurve = plotCurves_[ d.formula() ];

	plotCurve->attach( dplot_.get() );
    plotCurve->setLegendAttribute( QwtPlotCurve::LegendShowLine );
	plotCurve->setSymbol( new QwtSymbol( QwtSymbol::Style( QwtSymbol::Ellipse + (id % 10) ), Qt::NoBrush, QPen( Qt::darkMagenta ), QSize(5, 5 ) ) );
    plotCurve->setPen( QPen( colors[ id % 11 ] ) );
    plotCurve->setTitle( QString::fromStdWString( adcontrols::ChemicalFormula::formatFormula( d.formula() ) ) );

    std::shared_ptr< internal::SeriesData > ptr = d.shared_from_this();
    plotCurve->setData( new internal::xSeriesData( ptr ) );
	dplot_->setAxisScale( QwtPlot::xBottom
                          , d.boundingRect().left() - d.boundingRect().width() / 10.0
                          , d.boundingRect().right() + d.boundingRect().width() / 10.0);
	dplot_->setAxisScale( QwtPlot::yLeft, d.boundingRect().top(), d.boundingRect().bottom() + d.boundingRect().height() / 10.0);

	dplot_->zoomer().setZoomBase( true );
	dplot_->replot();
}

void
MSCalibSpectraWnd::plot_slope()
{
	rplot_->axisAutoScale( QwtPlot::xBottom );
	rplot_->axisAutoScale( QwtPlot::yLeft );
	rplot_->axisAutoScale( QwtPlot::yRight );

    if ( slopePlotCurve_ == 0 ) {
        slopePlotCurve_ = new QwtPlotCurve;
        slopePlotCurve_->attach( rplot_.get() );
        slopePlotCurve_->setLegendAttribute( QwtPlotCurve::LegendShowLine );
        slopePlotCurve_->setSymbol( new QwtSymbol( QwtSymbol::Style( QwtSymbol::Ellipse ), Qt::NoBrush, QPen( Qt::darkMagenta ), QSize(5, 5 ) ) );
        slopePlotCurve_->setPen( QPen( Qt::blue ) );
        slopePlotCurve_->setYAxis( QwtPlot::yLeft );
        slopePlotCurve_->setTitle( "slope" );
    }
	double minY = std::numeric_limits<double>::max();
	double maxY = std::numeric_limits<double>::min();
    QVector< QPointF > xy;
    for ( auto plot: plotData_ ) {
		minY = std::min( minY, plot.second->slope() );
		maxY = std::max( maxY, plot.second->slope() );
		xy << QPointF( std::sqrt( plot.second->exactMass() ), plot.second->slope() );
    }
    slopePlotCurve_->setSamples( xy );
    rplot_->zoomer().setZoomBase( true );
    
	// rplot_->setAxisScale( QwtPlot::yLeft, minY, maxY );
}

void
MSCalibSpectraWnd::plot_intercept()
{
    if ( interceptPlotCurve_ == 0 ) {
        interceptPlotCurve_ = new QwtPlotCurve;
        interceptPlotCurve_->attach( rplot_.get() );
        interceptPlotCurve_->setLegendAttribute( QwtPlotCurve::LegendShowLine );
        interceptPlotCurve_->setSymbol( new QwtSymbol( QwtSymbol::Style( QwtSymbol::Cross ), Qt::NoBrush, QPen( Qt::darkMagenta ), QSize(5, 5 ) ) );
        interceptPlotCurve_->setPen( QPen( Qt::red ) );
		interceptPlotCurve_->setYAxis( QwtPlot::yRight );
        interceptPlotCurve_->setTitle( "intercept" );
    }

	double minY = std::numeric_limits<double>::max();
	double maxY = std::numeric_limits<double>::min();
    QVector< QPointF > xy;
    for ( auto plot: plotData_ ) {
		minY = std::min( minY, plot.second->intercept() );
		maxY = std::max( maxY, plot.second->intercept() );        
        xy << QPointF( std::sqrt( plot.second->exactMass() ), plot.second->intercept() );
    }
    interceptPlotCurve_->setSamples( xy );
    rplot_->zoomer().setZoomBase( true );
	// rplot_->setAxisScale( QwtPlot::yRight, minY, maxY );
}

void
MSCalibSpectraWnd::plotTimeMarker( double t, double length )
{
	using namespace adcontrols::metric;

	double microseconds = scale_to_micro( t );
    time_length_marker_->setValue( length, microseconds );
	time_length_marker_->setLineStyle( QwtPlotMarker::Cross );
    time_length_marker_->setLinePen( Qt::gray, 0.0, Qt::DotLine );
    QwtText label( QString::fromStdString( (boost::format("%.4lfus")% microseconds).str() ) );
    time_length_marker_->setLabel( label );
	dplot_->replot();
}

void
MSCalibSpectraWnd::plotSelectedLengthTime( const std::wstring& formula )
{
    using namespace adcontrols::metric;

    auto it = plotData_.find( formula );
	if ( it != plotData_.end() ) {
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

            dplot_->setFooter( o.str().c_str() );
            if ( regressionCurve_ == 0 ) {
                regressionCurve_ = new QwtPlotCurve;
                regressionCurve_->attach( dplot_.get() );
            }
                
            const QRectF& rc = it->second->boundingRect();
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
            dplot_->setFooter( o.str() );
        }
        dplot_->replot();
    }
}

void
MSCalibSpectraWnd::generate_marged_result( Dataprocessor * /* processor */)
{
    margedSpectrum_ = std::make_shared< adcontrols::MassSpectrum >();
    margedCalibResult_ = std::make_shared< adcontrols::MSCalibrateResult >();
    adcontrols::MSAssignedMasses masses;

    size_t idSpectrum = 0;
    size_t nSeg = 0;
    number_of_segments_.clear();

    for ( auto result: results_ ) {

        auto ms = std::get<1>( result );
        if ( ! ms->isCentroid() )
            continue;

        auto calibResult = std::get<2>( result );
        double threshold = calibResult->threshold();
		const adcontrols::MSAssignedMasses& assigned = calibResult->assignedMasses();

        std::vector< size_t > index; // for each segment

        adcontrols::segment_wrapper<> segments( *ms );

        size_t idSegment = 0;
        for ( auto& fms: segments ) {
            for ( size_t i = 0; i < fms.size(); ++i ) {
                if ( fms.getIntensity( i ) > threshold )
                    index.push_back( i );
            }
            adcontrols::MassSpectrum clone;
            clone.clone( fms );
            clone.resize( index.size() );
            int n = 0;
            for ( auto idPeak: index ) {
                clone.setMass( n, fms.getMass( idPeak ) );
                clone.setTime( n, fms.getTime( idPeak ) );
                clone.setIntensity( n, fms.getIntensity( idPeak ) );
                if ( fms.getColorArray() )
                    clone.setColor( n, fms.getColor( idPeak ) );
                ++n;
            }
            if ( idSpectrum == 0 && idSegment == 0 )
                *margedSpectrum_ = clone;
            else
                margedSpectrum_->addSegment( clone );

            std::for_each( assigned.begin(), assigned.end(), [&]( const adcontrols::MSAssignedMass& t ){
					if ( t.idMassSpectrum() == idSegment ) {
                        adcontrols::MSAssignedMass x( t );
                        x.idMassSpectrum( nSeg + idSegment );
						x.idPeak( std::distance( index.begin(), std::find( index.begin(), index.end(), t.idPeak() ) ) );
                        masses << x;
                    }
                });

            ++idSegment;
		}
	
        number_of_segments_.push_back( idSegment ); // number of segments held in the data
        nSeg += idSegment;  // total number of segment
        ++idSpectrum; // next spectrum
    }

	margedCalibResult_->assignedMasses( masses );
	DataprocHandler::doAnnotateAssignedPeaks( *margedSpectrum_, masses );
    wndSpectra_[ 0 ] ->setData( margedSpectrum_, 0 );

#if 0
    portfolio::Folder folder = processor->getPortfolio().addFolder( L"MSCalibration" );
    portfolio::Folium folium = folder.findFoliumByName( L"Summary" );
    if ( folium.fail() )
        folium = folder.addFolium( L"Summary" );
    folium = *marged;
#endif    

}

