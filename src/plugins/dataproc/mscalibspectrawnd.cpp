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
#include <adcontrols/msassignedmass.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/mscalibration.hpp>
#include <adcontrols/mscalibrateresult.hpp>
#include <adcontrols/mscalibratemethod.hpp>
#include <adwplot/spectrumwidget.hpp>
#include <adportable/configuration.hpp>
#include <adportable/debug.hpp>
#include <adportable/array_wrapper.hpp>
#include <adplugin/manager.hpp>
#include <adplugin/widget_factory.hpp>
#include <adplugin/lifecycle.hpp>
#include <adplugin/lifecycleaccessor.hpp>
#include <adutils/processeddata.hpp>
#include <portfolio/folder.hpp>
#include <portfolio/folium.hpp>
#include <coreplugin/minisplitter.h>
#include <qwt_scale_widget.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_marker.h>
#include <QVBoxLayout>
#include <cmath>
#include <boost/format.hpp>

using namespace dataproc;

MSCalibSpectraWnd::MSCalibSpectraWnd( QWidget * parent ) : QWidget( parent )
                                                         , wndCalibSummary_( 0 )
                                                         , wndSplitter_( 0 )
                                                         , axis_( adwplot::SpectrumWidget::HorizontalAxisMass )
{
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
        
        for ( int i = 0; i < 4; ++i ) {

            std::shared_ptr< adwplot::SpectrumWidget > wnd = std::make_shared< adwplot::SpectrumWidget >(this);
            wnd->setAutoAnnotation( false );
            wnd->axisWidget( QwtPlot::yLeft )->scaleDraw()->setMinimumExtent( 50 );
            wnd->setMinimumHeight( 40 );
            wndSpectra_.push_back( wnd );
            wndSplitter_->addWidget( wnd.get() );
            if ( i )
                wnd->link( wndSpectra_[ i - 1 ].get() );

            std::shared_ptr< QwtPlotMarker > marker = std::make_shared< QwtPlotMarker >();
            markers_.push_back( marker );
            marker->attach( wnd.get() );
        }

        wndSpectra_[ 0 ]->link( wndSpectra_.back().get() );

        wndSplitter_->setOrientation( Qt::Vertical );

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

            if ( wndCalibSummary_ ) {
                adplugin::LifeCycleAccessor accessor( wndCalibSummary_ );
                adplugin::LifeCycle * p = accessor.get();
                if ( p )
                    p->OnInitialUpdate();

                connect( this, SIGNAL( fireSetData( const adcontrols::MSCalibrateResult&, const adcontrols::MassSpectrum& ) ),
                         wndCalibSummary_, SLOT( setData( const adcontrols::MSCalibrateResult&, const adcontrols::MassSpectrum& ) ) );
                
                splitter->addWidget( wndCalibSummary_ );
            }
        }

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
    size_t idx = 0;
    for ( auto& sp: spectra_ ) {
        markers_[ idx ]->setXValue( 0 );
        wndSpectra_[ idx++ ]->setData( sp, 0 );
        if ( idx >= wndSpectra_.size() )
            break;
    }
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
    portfolio::Folder folder = folium.getParentFolder();
	if ( ! ( folder && folder.name() == L"MSCalibration" ) )
		return;

    adportable::debug(__FILE__, __LINE__) << "handleSelectionChanged";

    spectra_.clear();
    folio_ = folder.folio();

    std::for_each( folio_.begin(), folio_.end(), [=]( portfolio::Folium& item ){

            if ( item.attribute( L"isChecked" ) == L"true" ) {
                
                processor->fetch( item );

                portfolio::Folio attachments = item.attachments();
                auto any = portfolio::Folium::find< adcontrols::MassSpectrumPtr >( attachments.begin(), attachments.end() );
                if ( any != attachments.end() ) {
                    adcontrols::MassSpectrumPtr ptr = boost::any_cast< adcontrols::MassSpectrumPtr >( *any );
                    spectra_.push_back( ptr );
                }
                
            }
        });

    size_t idx = 0;
    for ( auto& sp: spectra_ ) {
        wndSpectra_[ idx++ ]->setData( sp, 0 );
        if ( idx >= wndSpectra_.size() )
            break;
    }

    folium_ = folium;
    portfolio::Folio attachments = folium.attachments();
    portfolio::Folio::iterator it
        = portfolio::Folium::find<adcontrols::MassSpectrumPtr>(attachments.begin(), attachments.end());
    if ( it == attachments.end() )
        return;

    adutils::MassSpectrumPtr ptr = boost::any_cast< adutils::MassSpectrumPtr >( *it );

    // calib result
    it = portfolio::Folium::find<adutils::MSCalibrateResultPtr>(attachments.begin(), attachments.end());
    if ( it != attachments.end() ) {
        adutils::MSCalibrateResultPtr res = boost::any_cast< adutils::MSCalibrateResultPtr >( *it );
        emit fireSetData( *res, *ptr );
    }
    
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

    adplugin::LifeCycleAccessor accessor( wndCalibSummary_ );
    adplugin::LifeCycle * p = accessor.get();
    if ( p ) {
        adutils::MassSpectrumPtr ptr( new adcontrols::MassSpectrum );
        boost::any any( ptr );
        p->getContents( any );
        wndSpectra_[ 0 ]->setData( ptr, 0 );

		adcontrols::segment_wrapper<> segms( *ptr );
        
		double t = segms[ fcn ].getTime( idx );

        size_t nid = 0;
        std::for_each( spectra_.begin(), spectra_.end(), [&]( std::weak_ptr< adcontrols::MassSpectrum > wp ){
                if ( std::shared_ptr< adcontrols::MassSpectrum > sp = wp.lock() ) {
                    wndSpectra_[ nid ]->setData( sp, 0 );
                    int idx = assign_peaks::find_by_time( *sp, t, 3.0e-9 ); // 3ns tolerance
                    if ( idx >= 0 ) {
                        QwtPlotMarker& marker = *markers_[nid];
                        double x = ( axis_ == adwplot::SpectrumWidget::HorizontalAxisMass )
                            ? sp->getMass( idx )
                            : scale_to_micro( sp->getTime( idx ) );
                        marker.setValue( x, sp->getIntensity( idx ) );
                        marker.setLineStyle( QwtPlotMarker::Cross );
                        marker.setLinePen( Qt::gray, 0.0, Qt::DashDotLine );

                        QwtText label( QString::fromStdString( (boost::format("%.4lfus")% scale_to_micro( sp->getTime(idx) )).str() ) );
                        marker.setLabel( label );
                    }
                    ++nid;
                }
            });
    }
}

void
MSCalibSpectraWnd::handleValueChanged()
{
    adplugin::LifeCycleAccessor accessor( wndCalibSummary_ );
    adplugin::LifeCycle * p = accessor.get();
    if ( p ) {
        std::shared_ptr< adcontrols::MSAssignedMasses > assigned( std::make_shared< adcontrols::MSAssignedMasses >() );
        if ( readCalibSummary( *assigned ) ) {
            portfolio::Folium& folium = folium_;
            portfolio::Folio attachments = folium.attachments();
            
            // calib result
            portfolio::Folio::iterator it
                = portfolio::Folium::find<adcontrols::MSCalibrateResultPtr>(attachments.begin(), attachments.end());
            if ( it != attachments.end() ) {
                adutils::MSCalibrateResultPtr result = boost::any_cast< adutils::MSCalibrateResultPtr >( *it );
                result->assignedMasses( *assigned );
            }

            // retreive centroid spectrum
            it = portfolio::Folium::find<adcontrols::MassSpectrumPtr>(attachments.begin(), attachments.end());
            if ( it != attachments.end() ) {
                adutils::MassSpectrumPtr ptr = boost::any_cast< adutils::MassSpectrumPtr >( *it );
                if ( ptr->isCentroid() ) {
                    if ( DataprocHandler::doAnnotateAssignedPeaks( *ptr, *assigned ) )
                        replotSpectra();
                }
            }
        }
    }
}

int
MSCalibSpectraWnd::populate( std::vector< result_type >& vec)
{
    vec.clear();

    if ( folio_.empty() )
        return -1;

    size_t nCurId = 0;
    for ( portfolio::Folium::vector_type::iterator it = folio_.begin(); it != folio_.end(); ++it ) {

        portfolio::Folio attachments = it->attachments();

        adcontrols::MSCalibrateResultPtr res;
        portfolio::Folio::iterator atIt = portfolio::Folium::find< adcontrols::MSCalibrateResultPtr >( attachments.begin(), attachments.end() );
        if ( atIt != attachments.end() )
            res = boost::any_cast< adutils::MSCalibrateResultPtr >( *atIt );

        adcontrols::MassSpectrumPtr pms;
        portfolio::Folio::iterator msIt = portfolio::Folium::find< adcontrols::MassSpectrumPtr >( attachments.begin(), attachments.end() );
        if ( msIt != attachments.end() )
            pms = boost::any_cast< adutils::MassSpectrumPtr >( *msIt );

        vec.push_back( std::make_pair( res, pms ) );

        if ( it->id() == folium_.id() )
            nCurId = std::distance( folio_.begin(), it );
    }
    return nCurId;
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
        assert(0);
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
