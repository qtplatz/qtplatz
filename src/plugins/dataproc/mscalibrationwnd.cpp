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

#include "mscalibrationwnd.hpp"
#include "mainwindow.hpp"
#include "dataprocessor.hpp"
#include "dataprochandler.hpp"
#include <portfolio/folium.hpp>
#include <portfolio/folder.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/mscalibrateresult.hpp>
#include <adcontrols/msassignedmass.hpp>
#include <adwplot/spectrumwidget.hpp>
#include <adutils/processeddata.hpp>

#include <coreplugin/minisplitter.h>
#include <QBoxLayout>
#include <boost/any.hpp>
#include <adportable/configuration.hpp>
#include <adportable/debug.hpp>
#include <adplugin/lifecycle.hpp>
#include <adplugin/lifecycleaccessor.hpp>
#include <adplugin/manager.hpp>
#include <adplugin/widget_factory.hpp>
#include "qtwidgets_name.hpp"

using namespace dataproc;

namespace dataproc {
    
    class MSCalibrationWndImpl {
    public:
        ~MSCalibrationWndImpl() {}
        MSCalibrationWndImpl() : profileSpectrum_(0)
                               , processedSpectrum_(0)
                               , calibSummaryWidget_(0)  {
        }

        adwplot::SpectrumWidget * profileSpectrum_;
        adwplot::SpectrumWidget * processedSpectrum_;
        QWidget * calibSummaryWidget_;
        
        portfolio::Folium folium_;

    };
}

MSCalibrationWnd::MSCalibrationWnd( QWidget * parent ) : QWidget( parent )
{
    init();
}

void
MSCalibrationWnd::init()
{
    pImpl_.reset( new MSCalibrationWndImpl );
    Core::MiniSplitter * splitter = new Core::MiniSplitter;
    if ( splitter ) {
        // spectrum on top
        pImpl_->processedSpectrum_ = new adwplot::SpectrumWidget(this);
        splitter->addWidget( pImpl_->processedSpectrum_ );

        // summary table
        pImpl_->calibSummaryWidget_ = adplugin::widget_factory::create( L"qtwidgets2::MSCalibSummaryWidget" );
        if ( QWidget * pSummary = pImpl_->calibSummaryWidget_ ) {
            bool res;
            res = connect( pSummary, SIGNAL( currentChanged( size_t, size_t ) ), this, SLOT( handleSelSummary( size_t, size_t ) ) );
            assert(res);
            res = connect( pSummary, SIGNAL( valueChanged() ), this, SLOT( handleValueChanged() ) );
            assert(res);
            res = connect( pSummary, SIGNAL( on_recalibration_requested() ), this, SLOT( handle_recalibration_requested() ) );
            assert(res);
            res = connect( pSummary, SIGNAL( on_reassign_mass_requested() ), this, SLOT( handle_reassign_mass_requested() ) );
            assert(res);
            res = connect( pSummary, SIGNAL( on_apply_calibration_to_dataset() ), this, SLOT( handle_apply_calibration_to_dataset() ) );
            assert(res);
            res = connect( pSummary, SIGNAL( on_apply_calibration_to_default() ), this, SLOT( handle_apply_calibration_to_default() ) );
            assert(res);

            // Make a connection to zoomer in order to sync table in visible range
            res = connect( &pImpl_->processedSpectrum_->zoomer()
                           , SIGNAL( zoomed( const QRectF& ) ), pSummary, SLOT( handle_zoomed( const QRectF& ) ) );
            assert(res);

            res = connect( pImpl_->processedSpectrum_
                           , SIGNAL( onSelected( const QRectF& ) ), pSummary, SLOT( handle_selected( const QRectF& ) ) );
            assert(res);
            (void)res;

            adplugin::LifeCycleAccessor accessor( pSummary );
            adplugin::LifeCycle * p = accessor.get();
            if ( p )
                p->OnInitialUpdate();

            connect( this
                     , SIGNAL( fireSetData( const adcontrols::MSCalibrateResult&, const adcontrols::MassSpectrum& ) )
                     , pSummary, SLOT( setData( const adcontrols::MSCalibrateResult&, const adcontrols::MassSpectrum& ) ) );

            splitter->addWidget( pSummary );
        }

        splitter->setOrientation( Qt::Vertical );
    }

    QBoxLayout * toolBarAddingLayout = new QVBoxLayout( this );

    toolBarAddingLayout->setMargin(0);
    toolBarAddingLayout->setSpacing(0);
    toolBarAddingLayout->addWidget( splitter );
}

void
MSCalibrationWnd::handleSessionAdded( Dataprocessor * )
{
}

void
MSCalibrationWnd::handleSelectionChanged( Dataprocessor* processor, portfolio::Folium& folium )
{
    Q_UNUSED(processor);
	
    enum { idx_profile, idx_centroid };

    portfolio::Folder folder = folium.getParentFolder();

    if ( folder && folder.name() == L"MSCalibration" ) {

        pImpl_->folium_ = folium;

        boost::any& data = folium;
        // profile spectrum
        if ( adutils::ProcessedData::is_type< adutils::MassSpectrumPtr >( data ) ) { 
            adutils::MassSpectrumPtr ptr = boost::any_cast< adutils::MassSpectrumPtr >( data );
            pImpl_->processedSpectrum_->setData( *ptr, idx_profile );
        }

        portfolio::Folio attachments = folium.attachments();
        // centroid spectrum
        portfolio::Folio::iterator it = portfolio::Folium::find_first_of<adcontrols::MassSpectrumPtr>(attachments.begin()
                                                                                                      , attachments.end());
        if ( it == attachments.end() )
            return;

        adutils::MassSpectrumPtr ptr = boost::any_cast< adutils::MassSpectrumPtr >( *it );
        pImpl_->processedSpectrum_->setData( *ptr, idx_centroid );

        // calib result
        it = portfolio::Folium::find_first_of<adcontrols::MSCalibrateResultPtr>(attachments.begin(), attachments.end());
        if ( it != attachments.end() ) {
            adcontrols::MSCalibrateResultPtr calibResult = boost::any_cast< adutils::MSCalibrateResultPtr >( *it );
            emit fireSetData( *calibResult, *ptr );
        }

    }
}

void
MSCalibrationWnd::handleApplyMethod( const adcontrols::ProcessMethod& )
{
}

void
MSCalibrationWnd::handleSelSummary( size_t idx, size_t fcn )
{
	(void)idx; (void)fcn;

    adplugin::LifeCycleAccessor accessor( pImpl_->calibSummaryWidget_ );
    adplugin::LifeCycle * p = accessor.get();
    if ( p ) {
        adutils::MassSpectrumPtr ptr( new adcontrols::MassSpectrum );
        boost::any any( ptr );
        p->getContents( any );
        pImpl_->processedSpectrum_->setData( *ptr, 1 );
    }
}

void
MSCalibrationWnd::handleValueChanged()
{
    std::shared_ptr< adcontrols::MSAssignedMasses > assigned( new adcontrols::MSAssignedMasses );
    if ( readCalibSummary( *assigned ) ) {
        portfolio::Folium& folium = pImpl_->folium_;
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
                // update color & annotation
                if ( DataprocHandler::doAnnotateAssignedPeaks( *ptr, *assigned ) ) {
                    pImpl_->processedSpectrum_->setData( *ptr, 1 ); 
                }
            }
        }
    }
}

bool
MSCalibrationWnd::readCalibSummary( adcontrols::MSAssignedMasses& assigned )
{
    adplugin::LifeCycleAccessor accessor( pImpl_->calibSummaryWidget_ );
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
MSCalibrationWnd::handle_reassign_mass_requested()
{
    adcontrols::MSAssignedMasses assigned;
    if ( readCalibSummary( assigned ) ) {
        assert(0);
    }
}

void
MSCalibrationWnd::handle_recalibration_requested()
{
    adcontrols::MSAssignedMasses assigned;
    if ( readCalibSummary( assigned ) )
        MainWindow::instance()->applyCalibration( assigned );
}

void
MSCalibrationWnd::handle_apply_calibration_to_dataset()
{
    assert(0);
}

void
MSCalibrationWnd::handle_apply_calibration_to_default()
{
    assert(0);
}

