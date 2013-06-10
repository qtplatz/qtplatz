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

MSCalibrationWnd::MSCalibrationWnd( const adportable::Configuration& c
                                    , const std::wstring& apppath, QWidget * parent ) : QWidget( parent )
{
    init( c, apppath );
}

void
MSCalibrationWnd::init( const adportable::Configuration& c, const std::wstring& apppath )
{
	(void)apppath;
    using adportable::Configuration;
    
    pImpl_.reset( new MSCalibrationWndImpl );
    Core::MiniSplitter * splitter = new Core::MiniSplitter;
    if ( splitter ) {
        // spectrum on top
        pImpl_->processedSpectrum_ = new adwplot::SpectrumWidget(this);
        splitter->addWidget( pImpl_->processedSpectrum_ );

        // summary table
        const Configuration * pConfig = Configuration::find( c, L"MSCalibSummaryWidget" );
        if ( pConfig && pConfig->isPlugin() )
            pImpl_->calibSummaryWidget_ = adplugin::widget_factory::create( pConfig->_interface().c_str() );

        if ( ! pImpl_->calibSummaryWidget_ )
            pImpl_->calibSummaryWidget_ = adplugin::widget_factory::create( L"qtwidgets::MSCalibSummaryWidget" );

        bool res;
        res = connect( pImpl_->calibSummaryWidget_, SIGNAL( currentChanged( size_t ) ), this, SLOT( handleSelSummary( size_t ) ) );
        assert(res);

        res = connect( pImpl_->calibSummaryWidget_, SIGNAL( applyTriggered() ), this, SLOT( handleManuallyAssigned() ) );
        assert(res);

        res = connect( pImpl_->calibSummaryWidget_, SIGNAL( valueChanged() ), this, SLOT( handleValueChanged() ) );
        assert(res);
        (void)res;

        if ( pImpl_->calibSummaryWidget_ ) {
            adplugin::LifeCycleAccessor accessor( pImpl_->calibSummaryWidget_ );
            adplugin::LifeCycle * p = accessor.get();
            if ( p )
                p->OnInitialUpdate();

            connect( this, SIGNAL( fireSetData( const adcontrols::MSCalibrateResult&, const adcontrols::MassSpectrum& ) ),
                pImpl_->calibSummaryWidget_, SLOT( setData( const adcontrols::MSCalibrateResult&, const adcontrols::MassSpectrum& ) ) );

            splitter->addWidget( pImpl_->calibSummaryWidget_ );
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
MSCalibrationWnd::handleSelSummary( size_t idx )
{
    (void)idx;
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
MSCalibrationWnd::handleManuallyAssigned()
{
    adplugin::LifeCycleAccessor accessor( pImpl_->calibSummaryWidget_ );
    adplugin::LifeCycle * p = accessor.get();
    if ( p ) {
        boost::shared_ptr< adcontrols::MSAssignedMasses > assigned( new adcontrols::MSAssignedMasses );
        boost::any any( assigned );
        if ( p->getContents( any ) )
            MainWindow::instance()->applyCalibration( *assigned );
    }
}

void
MSCalibrationWnd::handleValueChanged()
{
    adplugin::LifeCycleAccessor accessor( pImpl_->calibSummaryWidget_ );
    adplugin::LifeCycle * p = accessor.get();
    if ( p ) {
        boost::shared_ptr< adcontrols::MSAssignedMasses > assigned( new adcontrols::MSAssignedMasses );
        boost::any any( assigned );
        if ( p->getContents( any ) ) {
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
                    // pImpl_->processedSpectrum_->setData( *ptr, 1 ); 
                }
            }
            // over write with current selected peak
            adutils::MassSpectrumPtr ptr( new adcontrols::MassSpectrum );
            boost::any any( ptr );
            p->getContents( any );  // got spectrum with size reduced by RA threshold
            pImpl_->processedSpectrum_->setData( *ptr, 1 ); 
            // todo: update annotation
        }
    }
}
