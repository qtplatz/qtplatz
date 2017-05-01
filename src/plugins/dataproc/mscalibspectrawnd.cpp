/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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
#include <adcontrols/msproperty.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/descriptions.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/scanlaw.hpp>
#include <adwidgets/mscalibratesummarytable.hpp>
#include <adplot/spectrumwidget.hpp>
#include <adportable/configuration.hpp>
#include <adlog/logger.hpp>
#include <adportable/array_wrapper.hpp>
#include <adportable/polfit.hpp>
#include <adportable/float.hpp>
#include <adportable/utf.hpp>
#include <adplugin/lifecycle.hpp>
#include <adplugin_manager/lifecycleaccessor.hpp>
#include <adplugin_manager/manager.hpp>
#include <adutils/processeddata.hpp>
#include <adportfolio/portfolio.hpp>
#include <adportfolio/folder.hpp>
#include <adportfolio/folium.hpp>
#include <qtwrapper/font.hpp>
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

        enum CurveType { CurveLinear = 2, CurveSquare, CurveQubic, CurveQuadratic };

        template<int curveType> struct Fitter {
            std::vector<double> x_, y_;
            std::vector<double> polynomial_;
            bool fitted_;

            Fitter() : fitted_( false ) {}
            void operator << ( const std::pair<double, double>& t ) { x_.push_back( t.first ); y_.push_back( t.second ); }
            void addXY( double x, double y ) { x_.push_back( x ); y_.push_back( y ); }
            bool fit() {
                return fitted_ = adportable::polfit::fit( x_.data(), y_.data(), x_.size(), curveType, polynomial_ );
            }
            double estimate_y( double x ) const {
                return adportable::polfit::estimate_y( polynomial_, x );
            }
            double standard_error() const {
                return adportable::polfit::standard_error( x_.data(), y_.data(), x_.size(), polynomial_ );                
            }
        };
}

using namespace dataproc;

MSCalibSpectraWnd::~MSCalibSpectraWnd()
{
}

namespace dataproc {
    
    static struct { const char * xbottom; const char * xtop; const char * yleft; const char * yright; } 
        axis_labels [] = {
            { "&radic;<span style=\"text-decoration: overline\">&nbsp;<i>m/z</i></span>"
              , 0
              , "time(&mu;s)"
              ,  0
            },
            { "flight length(m)" // x-bottom
              , 0
              , "time(&mu;s)"
              , 0
            },
            { "Slope(&mu;s/m)" // x-bottom
              , 0
              , "&radic;<span style=\"text-decoration: overline\">&nbsp;<i>m/z</i></span>" // y-left
              , 0
            },
            { "&radic;<span style=\"text-decoration: overline\">&nbsp;<i>m/z</i></span>"
              , 0
              , "time(&mu;s)"
              , 0
            }
        };
}

MSCalibSpectraWnd::MSCalibSpectraWnd( QWidget * parent ) : QWidget( parent )
                                                         , wndCalibSummary_( 0 )
                                                         , wndSplitter_( 0 )
                                                         , axis_( adplot::SpectrumWidget::HorizontalAxisMass )
{
    plots_.push_back( std::make_shared< adplot::plot >() ); // idPlotSqrtMassTime
    plots_.push_back( std::make_shared< adplot::plot >() ); // idPlotLengthTime
    plots_.push_back( std::make_shared< adplot::plot >() ); // idPlotOrbit
    plots_.push_back( std::make_shared< adplot::plot >() ); // idPlotInject

    for ( auto& plot: plots_ ) {
        plot->setMinimumHeight( 40 );
        plot->setMinimumWidth( 40 );
        QwtPlotGrid * grid = new QwtPlotGrid;
        grid->setMajorPen( Qt::gray, 0, Qt::DotLine );
        grid->attach( plot.get() );
    }

	QFont font = qtwrapper::font()( QFont(), qtwrapper::fontSizeSmall, qtwrapper::fontAxisTitle ); 

    int id = 0;
    for ( auto& plot: plots_ ) {
        auto& label = axis_labels[ id++ ];

        if ( label.xbottom ) {
            QwtText text( label.xbottom, QwtText::RichText );
            text.setFont( font );
            plot->setAxisTitle(QwtPlot::xBottom, text );
        }
        if ( label.xtop ) {
            QwtText text( label.xtop, QwtText::RichText );
            text.setFont( font );
            plot->setAxisTitle(QwtPlot::xTop, text );
            plot->enableAxis( QwtPlot::xTop );
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
        plot->axisAutoScale( QwtPlot::xTop );
        plot->axisAutoScale( QwtPlot::yLeft );
        plot->axisScaleEngine( QwtPlot::xBottom )->setAttribute( QwtScaleEngine::Floating, true );
        QwtPlotLegendItem * legendItem = new QwtPlotLegendItem;
        legendItem->attach( plot.get() );
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
            
            std::shared_ptr< adplot::SpectrumWidget > wnd = std::make_shared< adplot::SpectrumWidget >(this);
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
		if ( ( wndCalibSummary_ = new adwidgets::MSCalibrateSummaryTable() ) ) { // adplugin::widget_factory::create( L"qtwidgets2::MSCalibSummaryWidget" ) ) ) {
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
            res = connect( wndCalibSummary_, SIGNAL( on_apply_calibration_to_dataset()), this, SLOT( handle_apply_calibration_to_all() ) );
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
        if ( Core::MiniSplitter * splitter3 = new Core::MiniSplitter ) { // time vs length | slope, intercept vs m/z
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

		fullpath_ = processor->qfilename();

        if ( portfolio::Folium fSummary = fCalib.findFoliumByName( L"Summary Spectrum" ) ) {

			if ( fSummary.empty() )
				processor->fetch( fSummary );

			try {
				margedSpectrum_ = boost::any_cast< adcontrols::MassSpectrumPtr >( fSummary );
			} catch ( std::exception& ex ) {
				ADTRACE() << "got an exception: " << ex.what();
			}
            portfolio::Folio atts = fSummary.attachments();
            auto it = portfolio::Folium::find< adcontrols::MSCalibrateResultPtr >( atts.begin(), atts.end() );
            if ( it != atts.end() ) {
				try {
					margedCalibResult_ = boost::any_cast< adcontrols::MSCalibrateResultPtr >( *it );
				} catch ( std::exception& ex ) {
					ADTRACE() << "got an exception: " << ex.what();
				}
			}
        }
        if ( margedSpectrum_ ) {
            wndSpectra_[ 0 ] ->setData( margedSpectrum_, 0 );
            if ( margedCalibResult_ ) {
                emit onSetData( *margedCalibResult_, *margedSpectrum_ );
                flight_length_regression();
            }
        }
    }
}

void
MSCalibSpectraWnd::handleAxisChanged( adcontrols::hor_axis axis )
{
    axis_ = axis;
    for ( auto wnd: wndSpectra_ )
		wnd->setAxis( static_cast< adplot::SpectrumWidget::HorizontalAxis >( axis ) );
    
    replotSpectra();
}

void
MSCalibSpectraWnd::replotSpectra()
{
    if ( margedSpectrum_ ) {
        wndSpectra_[ 0 ]->setTitle( QString("Coadded spectrum") );
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
    curves_.clear();

	if ( ! margedCalibResult_ )
		return;

    const adcontrols::MSAssignedMasses& masses = margedCalibResult_->assignedMasses();
    adcontrols::segment_wrapper<> segments( *margedSpectrum_ );

    enum { calibrant_assined_mass, calibrant_sqrt_m, calibrant_time, calibrant_length, calibrant_corrected_time };

	typedef std::tuple< adcontrols::MSAssignedMass, double, double, double, double > calibrant_t;
    std::map< int, std::vector< calibrant_t > > time_corrected_calibrants;

    if ( auto spectrometer = adcontrols::MassSpectrometer::create( segments[0].getMSProperty().dataInterpreterClsid() ) ) {

        spectrometer->setAcceleratorVoltage( segments[0].getMSProperty().acceleratorVoltage(), segments[0].getMSProperty().tDelay() );

        if ( auto scanLaw = spectrometer->scanLaw() ) {

            for ( auto& pk: masses ) {

                auto& ms = segments[ pk.idMassSpectrum() ];

                if ( pk.enable() )
                    time_corrected_calibrants[ pk.mode() ].push_back( std::make_tuple( pk
                                                                                       , std::sqrt( pk.exactMass() )
                                                                                       , adcontrols::metric::scale_to_micro( pk.time() )
                                                                                       , scanLaw->fLength( pk.mode() )
                                                                                       , adcontrols::metric::scale_to_micro( pk.time() ) ) );
            }
        }
    }

    int id = 0;
    std::map< int, Fitter<CurveLinear> > time_correctors;

    for ( auto& map: time_corrected_calibrants ) {
        
        Fitter<CurveLinear>& time_corrector = time_correctors[ map.first ];

        for ( auto& calibrant: map.second )
            time_corrector << std::make_pair( std::get< calibrant_sqrt_m >( calibrant ), std::get< calibrant_time >( calibrant ) );

        time_corrector.fit();
    }

    // plot
    for( auto& time_corrector: time_correctors )
        plot_time_corrected_calibrant( id++, time_corrector.first, time_corrector.second, *plots_[ idPlotSqrtMassTime ] );

    // length, time plot for each ion

    std::map< std::wstring, std::pair< double, Fitter< CurveLinear > > > formula_length_time_fitters;

    for ( const auto& map: time_corrected_calibrants ) {
        
        for ( auto& calibrant: map.second ) {
            const adcontrols::MSAssignedMass pk = std::get< calibrant_assined_mass >( calibrant );
			Fitter<CurveLinear>& fitter = formula_length_time_fitters[ pk.formula() ].second;
            
            formula_length_time_fitters[ pk.formula() ].first = std::get< calibrant_sqrt_m >( calibrant );
            fitter << std::make_pair( std::get< calibrant_length >( calibrant ), std::get< calibrant_time >( calibrant ) );
        }
    }

    int nfitted = 0;
    id = 0;
    for ( auto& fitter: formula_length_time_fitters ) {
		if ( fitter.second.second.fit() )
            ++nfitted;
        plot_length_time( id++, fitter.first /* formula */, fitter.second.second, *plots_[ idPlotLengthTime ] );
    }

    if ( nfitted <= 1 )
        return;

    // calculate orbital_ and injection_ sector calibration (sqrt(m), slope) and (sqrt(m), intercept)
    Fitter< CurveLinear > orbital_sector_fitter, injection_sector_fitter;

    for ( const auto& formula_length_time: formula_length_time_fitters ) {
        const Fitter< CurveLinear >& fitter = formula_length_time.second.second;
        double sqrt_m = formula_length_time.second.first;

        if ( fitter.fitted_ ) {
            injection_sector_fitter << std::make_pair( sqrt_m, fitter.polynomial_[ 0 ] );
            orbital_sector_fitter << std::make_pair( fitter.polynomial_[ 1 ], sqrt_m );
        }
    }

    if ( orbital_sector_fitter.fit() && injection_sector_fitter.fit() ) {
        using namespace adcontrols;
        MSCalibration calib( orbital_sector_fitter.polynomial_, micro, injection_sector_fitter.polynomial_, MSCalibration::MULTITURN_NORMALIZED ); 
        margedCalibResult_->calibration( calib );
    }

    plot_injection_sector_calibration( 0, injection_sector_fitter, *plots_[ idPlotInjection ] );
    plot_orbital_sector_calibration( 1, orbital_sector_fitter, *plots_[ idPlotOrbit ] );
}

void
MSCalibSpectraWnd::handleCheckStateChanged( Dataprocessor* processor, portfolio::Folium& folium, bool isChecked )
{
    (void)processor;    (void)isChecked;

	using adcontrols::MSCalibration;
    using adcontrols::metric::micro;

    portfolio::Folder folder = folium.parentFolder();
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
	
	populate( processor, folder );
    generate_marged_result( processor );
	flight_length_regression();

    if ( margedCalibResult_ && margedSpectrum_ )
        emit onSetData( *margedCalibResult_, *margedSpectrum_ );
    
    if ( margedSpectrum_ )
        wndSpectra_[ 0 ] ->setData( margedSpectrum_, 0 );
}

void
MSCalibSpectraWnd::handleProcessed( Dataprocessor* processor, portfolio::Folium& folium )
{
    handleSelectionChanged( processor, folium );
}

void
MSCalibSpectraWnd::handleSelectionChanged( Dataprocessor* processor, portfolio::Folium& folium )
{
    (void)processor;
    using adcontrols::MSCalibration;
    using adcontrols::metric::micro;

    portfolio::Folder folder = folium.parentFolder();
	if ( ! ( folder && folder.name() == L"MSCalibration" ) )
		return;

    fullpath_ = processor->qfilename();
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
        double x = ( axis_ == adplot::SpectrumWidget::HorizontalAxisMass )
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
            //plotSelectedLengthTime( it->formula() );
        }
    }

    size_t nwnd = 1;
    for( auto& selected: selSpectra_ ) {
        if ( auto sp = selected.second.lock() ) {
            int idx = assign_peaks::find_by_time( *sp, time, 3.0e-9 );
            if ( idx >= 0 ) {
                QwtPlotMarker& marker = *markers_[nwnd];
                double x = ( axis_ == adplot::SpectrumWidget::HorizontalAxisMass )
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

    using namespace adcontrols;

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

        const auto& prop = margedSpectrum_->getMSProperty();

        if ( auto spectrometer = adcontrols::MassSpectrometer::create( prop.dataInterpreterClsid() ) ) {
            spectrometer->setAcceleratorVoltage( prop.acceleratorVoltage(), prop.tDelay() );
            
            if ( auto scanLaw = spectrometer->scanLaw() ) {

                adcontrols::ComputeMass< adcontrols::ScanLaw > mass_calculator( *scanLaw, calib );
                for ( auto& a: assigned ) {
                    double mass = mass_calculator( a.time(), a.mode() );
                    a.mass( mass );
                    segments[ a.idMassSpectrum() ].setMass( a.idPeak(), mass );
                }
                DataprocHandler::doAnnotateAssignedPeaks( *margedSpectrum_, assigned );
                margedCalibResult_->assignedMasses( assigned );

                emit onSetData( *margedCalibResult_, *margedSpectrum_ );
            }
            
        }

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
    if ( margedCalibResult_ ) {
        if ( Dataprocessor * processor = SessionManager::instance()->getActiveDataprocessor() ) {
            std::wstring clsid = adportable::utf::to_wstring( margedSpectrum_->getMSProperty().dataInterpreterClsid() );
            processor->applyCalibration( clsid, *margedCalibResult_ );
            MainWindow::instance()->dataMayChanged(); // notify for dockwidgets
        }
    }
}

void
MSCalibSpectraWnd::handle_apply_calibration_to_all()
{
    if ( margedCalibResult_ ) {
        std::wstring clsid = adportable::utf::to_wstring( margedSpectrum_->getMSProperty().dataInterpreterClsid() );

        for ( auto& session: *SessionManager::instance() )
            session.processor()->applyCalibration( clsid, *margedCalibResult_ );

        MainWindow::instance()->dataMayChanged(); // notify for dockwidgets
    }
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
	rc.setHeight( drawRect.height() / 4 );
    rc.setWidth( drawRect.width() * 0.6 );
    for ( auto& plot: plots_ ) {
        renderer.render( plot.get(), &painter, rc );
        rc.moveTo( rc.left(), rc.bottom() );
    }

    // renderer.render( pImpl_->processedSpectrum_, &painter, drawRect );
    // ---------- calibratin equation ----------
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

        auto clone = std::make_shared< adcontrols::MassSpectrum >();
        clone->clone( segments[0] );
        clone->resize( calibResult->assignedMasses().size() );

        // reconstract mass spectrum and corresponding MSAssignedMasses
        int idPeak = 0;
        for ( auto& t: calibResult->assignedMasses() ) {

			auto& fms = segments[ t.idMassSpectrum() ];
            clone->setMass( idPeak, fms.getMass( t.idPeak() ) );
            clone->setIntensity( idPeak, fms.getIntensity( t.idPeak() ) );
            clone->setTime( idPeak, fms.getTime( t.idPeak() ) );
			if ( const unsigned char * colors = fms.getColorArray() )
                clone->setColor( idPeak, colors[ t.idPeak() ] );

            adcontrols::MSAssignedMass x( t );
            x.idMassSpectrum( static_cast<uint32_t>(idSpectrum) );
            x.idPeak( idPeak );
            masses << x;
            ++idPeak;
        }
        if ( idSpectrum == 0 )
            *margedSpectrum_ = *clone;
        else
            *margedSpectrum_ << std::move( clone );
        ++idSpectrum; // move on to next spectrum
    }
    margedSpectrum_->addDescription( adcontrols::description( L"Calibration", L"Coadded Summary" ) );
	margedCalibResult_->assignedMasses( masses );
    if ( masses.size() > 0 )
        DataprocHandler::doAnnotateAssignedPeaks( *margedSpectrum_, masses );
}


static Qt::GlobalColor colors [] = {
    Qt::blue, Qt::red, Qt::green, Qt::cyan, Qt::magenta, Qt::yellow
    , Qt::darkRed, Qt::darkGreen, Qt::darkBlue, Qt::darkCyan, Qt::darkMagenta
};

void
MSCalibSpectraWnd::plot_time_corrected_calibrant( int id, int mode, const Fitter<CurveLinear>& fitter, adplot::plot& plot )
{
    QwtText title( (boost::format("laps: %d") % mode ).str().c_str() ); 
    plot_fitter( id, title, fitter, plot );

    plot.axisAutoScale( QwtPlot::xBottom );
    plot.axisAutoScale( QwtPlot::yLeft );
	plot.replot();
}

void
MSCalibSpectraWnd::plot_length_time( int id, const std::wstring& formula, const Fitter<CurveLinear>& fitter, adplot::plot& plot )
{
    QwtText text( QString::fromStdWString( adcontrols::ChemicalFormula::formatFormula( formula ) ), QwtText::RichText );
    plot_fitter( id, text, fitter, plot );

    plot.axisAutoScale( QwtPlot::xBottom );
    plot.axisAutoScale( QwtPlot::yLeft );
	plot.replot();
}

void
MSCalibSpectraWnd::plot_orbital_sector_calibration( int id, const Fitter<CurveLinear>& fitter, adplot::plot& plot )
{
    plot_fitter( id, QwtText("orbital sector"), fitter, plot );
    std::ostringstream o;
    o << "&radic;<span style=\"text-decoration: overline\">&nbsp;<i>m/z</i></span> = ";
    const std::vector<double> coeffs = fitter.polynomial_;
    o << boost::format( "%.7e + " ) % coeffs[0];
    o << boost::format( "%.7e&sdot;t" ) % coeffs[1];
    for ( size_t i = 2; i < coeffs.size(); ++i )
        o << boost::format( " + %.7e&sdot;t<sup>%d</sup>") % coeffs[i] % i;
    plot.setFooter( o.str() );

    plot.axisAutoScale( QwtPlot::xBottom );
    plot.axisAutoScale( QwtPlot::yLeft );
	plot.replot();
}

void
MSCalibSpectraWnd::plot_injection_sector_calibration( int id, const Fitter<CurveLinear>& fitter, adplot::plot& plot )
{
    plot_fitter( id, QwtText("injection sector"), fitter, plot );
    std::ostringstream o;
    o << "time(&mu;s) = ";
    const std::vector<double> coeffs = fitter.polynomial_;
    o << boost::format( "%.7e + " ) % coeffs[0];
    o << boost::format( "%.7e&sdot;&radic;<span style=\"text-decoration: overline\">&nbsp;<i>m/z</i></span>" ) % coeffs[1];
    for ( size_t i = 2; i < coeffs.size(); ++i )
        o << boost::format( " + %.7e&sdot;t<sup>%d</sup>") % coeffs[i] % i;
    plot.setFooter( o.str() );

    plot.axisAutoScale( QwtPlot::xBottom );
    plot.axisAutoScale( QwtPlot::yLeft );
	plot.replot();
}

void
MSCalibSpectraWnd::plot_fitter( int id, const QwtText& title, const Fitter<2>& fitter, adplot::plot& plot, int xAxis, int yAxis )
{
    curves_.push_back( std::make_shared< QwtPlotCurve >() );
    auto curve = curves_.back();

    curve->setLegendAttribute( QwtPlotCurve::LegendShowLine );
	QPen pen( colors[ id % 11 ] );
	curve->setSymbol( new QwtSymbol( QwtSymbol::Style( QwtSymbol::Ellipse + (id % 10) ), Qt::NoBrush, pen, QSize(5, 5) ) );
	curve->setPen( pen );
    curve->setTitle( title );
    curve->setStyle( QwtPlotCurve::NoCurve );
    if ( xAxis )
        curve->setXAxis( xAxis );
    if ( yAxis )
        curve->setYAxis( yAxis );

    QVector< QPointF > xy;
    for ( size_t i = 0; i < fitter.x_.size(); ++i )
		xy.push_back( QPointF( fitter.x_[i], fitter.y_[i] ) );

    curve->setSamples( xy );
    curve->attach( &plot );

    double x1 = *std::max_element( fitter.x_.begin(), fitter.x_.end() );

    // plot regression line
    curves_.push_back( std::make_shared< QwtPlotCurve >() );
    auto regression_curve = curves_.back();

    double y0 = fitter.estimate_y( 0.0 );
    double y1 = fitter.estimate_y( x1 );
    QVector< QPointF > line;
    line.push_back( QPointF( 0.0, y0 ) );
    line.push_back( QPointF( x1, y1 ) );
    regression_curve->setSamples( line );
    regression_curve->setPen( pen );
    if ( xAxis )
        regression_curve->setXAxis( xAxis );
    if ( yAxis )
        regression_curve->setYAxis( yAxis );
    regression_curve->attach( &plot );
}


