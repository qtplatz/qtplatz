/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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
#include <adplugin/lifecycle.hpp>
#include <adplugin/lifecycleaccessor.hpp>
#include <adutils/processeddata.hpp>
#include <portfolio/folder.hpp>
#include <portfolio/folium.hpp>
#include <coreplugin/minisplitter.h>
#include <QVBoxLayout>
#include <boost/foreach.hpp>
#include <cmath>

using namespace dataproc;

MSCalibSpectraWnd::MSCalibSpectraWnd( const adportable::Configuration& c
                                      , const std::wstring& apppath
                                      , QWidget * parent ) : QWidget( parent )
                                                           , wndCalibSummary_( 0 )
                                                           , wndSplitter_( 0 )
{
    init( c, apppath );
}

void
MSCalibSpectraWnd::init( const adportable::Configuration& c, const std::wstring& apppath )
{
    (void)c;
    (void)apppath;

    using adportable::Configuration;

    Core::MiniSplitter * splitter = new Core::MiniSplitter;  // spectra | table
    if ( splitter ) {
        // spectrum on left
        wndSplitter_ = new Core::MiniSplitter;
        splitter->addWidget( wndSplitter_ );
        
        for ( int i = 0; i < 5; ++i ) {
            boost::shared_ptr< adwplot::SpectrumWidget > wnd( new adwplot::SpectrumWidget(this) );
            wnd->setMinimumHeight( 40 );
            wndSpectra_.push_back( wnd );
            wndSplitter_->addWidget( wnd.get() );
            if ( i )
                wnd->link( wndSpectra_[ i - 1 ].get() );
        }
        wndSpectra_[ 0 ]->link( wndSpectra_.back().get() );

        wndSplitter_->setOrientation( Qt::Vertical );

        // summary table
        const Configuration * pConfig = Configuration::find( c, L"MSMCalibSummaryWidget" );
        if ( pConfig && pConfig->isPlugin() )
            wndCalibSummary_ = adplugin::manager::widget_factory( *pConfig, apppath.c_str(), 0 );

        bool res;
        res = connect( wndCalibSummary_, SIGNAL( currentChanged( size_t ) ), this, SLOT( handleSelSummary( size_t ) ) );
        // assert( res );
        res = connect( wndCalibSummary_, SIGNAL( applyTriggered() ), this, SLOT( handleManuallyAssigned() ) );
        assert( res );
        res = connect( wndCalibSummary_, SIGNAL( valueChanged() ), this, SLOT( handleValueChanged() ) );
        assert( res );
        res = connect( wndCalibSummary_, SIGNAL( applyPeakAssign() ), this, SLOT( handleUpdatePeakAssign() ) );
        assert( res );
        if ( wndCalibSummary_ ) {
            adplugin::LifeCycleAccessor accessor( wndCalibSummary_ );
            adplugin::LifeCycle * p = accessor.get();
            if ( p )
                p->OnInitialUpdate();

            connect( this, SIGNAL( fireSetData( const adcontrols::MSCalibrateResult&, const adcontrols::MassSpectrum& ) ),
                     wndCalibSummary_, SLOT( setData( const adcontrols::MSCalibrateResult&, const adcontrols::MassSpectrum& ) ) );

            splitter->addWidget( wndCalibSummary_ );
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
MSCalibSpectraWnd::handleSelectionChanged( Dataprocessor* processor, portfolio::Folium& folium )
{
    Q_UNUSED(processor);
    Q_UNUSED(folium);

    portfolio::Folder folder = folium.getParentFolder();
    if ( folder && folder.name() != L"MSCalibration" )
        return;

    do {
        folio_ = folder.folio();
        for ( portfolio::Folium::vector_type::iterator it = folio_.begin(); it != folio_.end(); ++it )
			processor->fetch( *it );
        
        portfolio::Folium::vector_type::iterator it = folio_.begin();
        while ( it->id() != folium.id() && it != folio_.end() )
            ++it;
        spectra_.clear();
        size_t idx = 0;
        for ( ; it != folio_.end() && idx < wndSpectra_.size(); ++it, ++idx ) {
			portfolio::Folio attachments = it->attachments();
            portfolio::Folio::iterator msIt
                = portfolio::Folium::find_first_of<adcontrols::MassSpectrumPtr>(attachments.begin(), attachments.end());
            if ( msIt != attachments.end() ) {
                adutils::MassSpectrumPtr ptr = boost::any_cast< adutils::MassSpectrumPtr >( *msIt );
                wndSpectra_[ idx ]->setData( *ptr );
                spectra_.push_back( ptr );
            }
        }
    } while( 0 );

    folium_ = folium;
    portfolio::Folio attachments = folium.attachments();
    portfolio::Folio::iterator it
        = portfolio::Folium::find_first_of<adcontrols::MassSpectrumPtr>(attachments.begin(), attachments.end());
    if ( it == attachments.end() )
        return;
    adutils::MassSpectrumPtr ptr = boost::any_cast< adutils::MassSpectrumPtr >( *it );
    // calib result
    it = portfolio::Folium::find_first_of<adutils::MSCalibrateResultPtr>(attachments.begin(), attachments.end());
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
MSCalibSpectraWnd::handleSelSummary( size_t idx )
{
    adplugin::LifeCycleAccessor accessor( wndCalibSummary_ );
    adplugin::LifeCycle * p = accessor.get();
    if ( p ) {
        adutils::MassSpectrumPtr ptr( new adcontrols::MassSpectrum );
        boost::any any( ptr );
        p->getContents( any );
        wndSpectra_[ 0 ]->setData( *ptr );

        double t = ptr->getTime( idx );
        adportable::debug(__FILE__, __LINE__) << "handleSelSummary(" << idx << ") t=" << t;

        size_t nid = 0;
        BOOST_FOREACH( adutils::MassSpectrumPtr p, spectra_ ) {
            if ( nid ) {
                int idx = assign_peaks::find_by_time( *p, t, 3.0e-9 ); // 3ns tolerance
                if ( idx >= 0 ) {
                    adutils::MassSpectrumPtr tmp( new adcontrols::MassSpectrum( *p ) );
                    tmp->setColor( idx, 2 );
                    wndSpectra_[ nid ]->setData( *tmp );
                } else {
                    wndSpectra_[ nid ]->setData( *p ); // clear color
                }
            }
            ++nid;
        }

    }
}

//void
//MSCalibSpectraWnd::applyAssigned( const adcontrols::MSAssignedMasses&, const portfolio::Folium& )
//{
//}

void
MSCalibSpectraWnd::handleManuallyAssigned()
{
    // trigger apply calibration
    adplugin::LifeCycleAccessor accessor( wndCalibSummary_ );
    adplugin::LifeCycle * p = accessor.get();
    if ( p ) {
        boost::shared_ptr< adcontrols::MSAssignedMasses > assigned( new adcontrols::MSAssignedMasses );
        boost::any any( assigned );
        if ( p->getContents( any ) ) {
            MainWindow::instance()->applyCalibration( *assigned, folium_ );
        }
    }
}

void
MSCalibSpectraWnd::handleValueChanged()
{
    adplugin::LifeCycleAccessor accessor( wndCalibSummary_ );
    adplugin::LifeCycle * p = accessor.get();
    if ( p ) {
        boost::shared_ptr< adcontrols::MSAssignedMasses > assigned( new adcontrols::MSAssignedMasses );
        boost::any any( assigned );
        if ( p->getContents( any ) ) {
            portfolio::Folium& folium = folium_;
            portfolio::Folio attachments = folium.attachments();
            
            // calib result
            portfolio::Folio::iterator it
                = portfolio::Folium::find_first_of<adcontrols::MSCalibrateResultPtr>(attachments.begin(), attachments.end());
            if ( it != attachments.end() ) {
                adutils::MSCalibrateResultPtr result = boost::any_cast< adutils::MSCalibrateResultPtr >( *it );
                result->assignedMasses( *assigned );
            }

            // retreive centroid spectrum
            it = portfolio::Folium::find_first_of<adcontrols::MassSpectrumPtr>(attachments.begin(), attachments.end());
            if ( it != attachments.end() ) {
                adutils::MassSpectrumPtr ptr = boost::any_cast< adutils::MassSpectrumPtr >( *it );
                if ( ptr->isCentroid() ) {
                    // replace centroid spectrum with colored
                    std::vector< unsigned char > color_table( ptr->size() );
                    memset( color_table.data(), 0, color_table.size() );
                    const unsigned char * colors = ptr->getColorArray();
                    if ( colors ) 
                        std::copy( colors, colors + ptr->size(), color_table.begin() );

                    using adcontrols::MSAssignedMasses;
                    
                    for ( MSAssignedMasses::vector_type::const_iterator it = assigned->begin(); it != assigned->end(); ++it ) {
                        if ( ! it->formula().empty() )
                            color_table[ it->idMassSpectrum() ] = 1;
                        else
                            color_table[ it->idMassSpectrum() ] = 0;
                    }
                    ptr->setColorArray( color_table.data() );
                }
            }
            // over write with current selected peak
            adutils::MassSpectrumPtr ptr( new adcontrols::MassSpectrum );
            boost::any any( ptr );
            p->getContents( any );  // got spectrum with size reduced by RA threshold
            wndSpectra_[ 0 ]->setData( *ptr ); 
            // todo: update annotation
        }
    }
}

void
MSCalibSpectraWnd::doCalibration( adcontrols::MassSpectrum& centroid, adcontrols::MSCalibrateResult& res, const adcontrols::MSAssignedMasses& assigned )
{
    adcontrols::ProcessMethod m;
    MainWindow::instance()->getProcessMethod( m );
    const adcontrols::MSCalibrateMethod * mcalib = m.find< adcontrols::MSCalibrateMethod >();

    if ( DataprocHandler::doMSCalibration( res, centroid, *mcalib, assigned ) ) {
    }

    // replace calibration
    //if ( DataprocessorImpl::applyMethod( folium, *mcalib, assigned ) )
    //SessionManager::instance()->updateDataprocessor( this, folium );
}

void
MSCalibSpectraWnd::handleUpdatePeakAssign()
{
    std::vector< result_type > result_spectra;
    int nCurr = populate( result_spectra );
    if ( nCurr < 0 )
        return;

    assign_peaks assigner( 10.0e-9, 0.0 );

    // it should make up marged 'assined masses' data

    for ( size_t i = 0; i < result_spectra.size(); ++i ) {

        adcontrols::MSAssignedMasses assigned; // assined to current spectrum (spectra should be ordered by ejection time)
        for ( size_t k = 0; k <= i; ++k ) {
            if ( result_spectra[ k ].first )
                assigned += result_spectra[ k ].first->assignedMasses();
        }

        if ( i != size_t( nCurr ) ) {
            if ( result_spectra[ i ].first && result_spectra[ i ].second ) {
                adcontrols::MSAssignedMasses res;
                assigner( res, *result_spectra[ i ].second, assigned );
                result_spectra[ i ].first->assignedMasses( res );
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
        portfolio::Folio::iterator atIt = portfolio::Folium::find_first_of< adcontrols::MSCalibrateResultPtr >( attachments.begin(), attachments.end() );
        if ( atIt != attachments.end() )
            res = boost::any_cast< adutils::MSCalibrateResultPtr >( *atIt );

        adcontrols::MassSpectrumPtr pms;
        portfolio::Folio::iterator msIt = portfolio::Folium::find_first_of< adcontrols::MassSpectrumPtr >( attachments.begin(), attachments.end() );
        if ( msIt != attachments.end() )
            pms = boost::any_cast< adutils::MassSpectrumPtr >( *msIt );

        vec.push_back( std::make_pair( res, pms ) );

        if ( it->id() == folium_.id() )
            nCurId = std::distance( folio_.begin(), it );
    }
    return nCurId;
}

