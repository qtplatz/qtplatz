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

#include "msspectrawnd.hpp"
#include "mainwindow.hpp"
#include "dataprocessor.hpp"
#include "selchanged.hpp"
#include "sessionmanager.hpp"
#include "dataproc_document.hpp"
#include "qtwidgets_name.hpp"

#include <adcontrols/description.hpp>
#include <adcontrols/datafile.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msqpeaks.hpp>
#include <adutils/processeddata.hpp>
#include <adlog/logger.hpp>
#include <adplugin/lifecycle.hpp>
#include <adplugin/plugin.hpp>
#include <adplugin/plugin_ptr.hpp>
#include <adportable/configuration.hpp>
#include <adplot/chromatogramwidget.hpp>
#include <adplot/spectrumwidget.hpp>
#include <adplot/spectrogramwidget.hpp>
#include <adplot/peakmarker.hpp>
#include <adwidgets/msquantable.hpp>

#include <portfolio/folium.hpp>
#include <portfolio/folder.hpp>
#include <portfolio/portfolio.hpp>
#include <boost/variant.hpp>
#include <boost/any.hpp>
#include <boost/exception/all.hpp>
#include <qtwrapper/qstring.hpp>
#include <coreplugin/minisplitter.h>
#include <QBoxLayout>

namespace dataproc {

    class MSSpectraWnd::impl {
    public:
        impl( MSSpectraWnd * p ) : pThis_( p )
                                 , table_( std::make_shared< adwidgets::MSQuanTable >() )
                                 , isTimeAxis_( false )
                                 , dirty_( false ) {
            
            for ( size_t i = 0; i < plots_.size(); ++i ) {

                plots_[ i ] = std::make_shared< adplot::SpectrumWidget >();
                markers_[ i ] = std::make_shared< adplot::PeakMarker >();

            }
            
        }
        
        ~impl() {
        }

        MSSpectraWnd * pThis_;

        std::map< std::wstring // folium (profile) Guid (attGuid)
                  , std::tuple<int                                         // 0 idx
                               , std::wstring                              // 1 attached (:= centroid) guid
                               , std::weak_ptr< adcontrols::MassSpectrum>  // 2 
                               , std::wstring                              // 3 filename::folium.name
                               >  > dataIds_;

        std::pair< std::wstring, std::weak_ptr< adcontrols::MassSpectrum > > profile_;

        std::shared_ptr< adwidgets::MSQuanTable > table_;
        std::array< std::shared_ptr< adplot::SpectrumWidget >, 2 > plots_;
        std::array< std::shared_ptr< adplot::PeakMarker >, 2 > markers_;
        bool isTimeAxis_;
        bool dirty_;
        
    };
    
}

using namespace dataproc;

MSSpectraWnd::~MSSpectraWnd()
{
    delete impl_;
}

MSSpectraWnd::MSSpectraWnd( QWidget *parent ) : QWidget(parent)
                                              , impl_( new impl( this ) )
{
    init();
}

void
MSSpectraWnd::init()
{
    Core::MiniSplitter * splitter = new Core::MiniSplitter;
    if ( splitter ) {

        if ( auto lifecycle = qobject_cast< adplugin::LifeCycle * >( impl_->table_.get() ) ) {
            lifecycle->OnInitialUpdate();
        }

        for ( size_t i = 0; i < impl_->plots_.size(); ++i ) {

            auto& plot = impl_->plots_[i];
            auto& marker = impl_->markers_[i];

            connect( plot.get()
                     , static_cast< void(adplot::SpectrumWidget::*)(const QRectF&)>(&adplot::SpectrumWidget::onSelected)
                     , [=] ( const QRectF& rc ) { impl_->table_->handleSelected( rc, impl_->isTimeAxis_ ); } );

            plot->enableAxis( QwtPlot::yRight );
            marker->attach( plot.get() );
            marker->visible( true );
            marker->setYAxis( QwtPlot::yRight );

            if ( i )
                impl_->plots_[ 0 ]->link( plot.get() );

            splitter->addWidget( plot.get() );

        }

        splitter->addWidget( impl_->table_.get() );
        splitter->setOrientation( Qt::Vertical );
    }

    QBoxLayout * toolBarAddingLayout = new QVBoxLayout( this );
    toolBarAddingLayout->setMargin(0);
    toolBarAddingLayout->setSpacing(0);
    toolBarAddingLayout->addWidget( splitter );

    connect( impl_->table_.get()
             , static_cast<void (adwidgets::MSQuanTable::*)(const QString&, int, int)>(&adwidgets::MSQuanTable::currentChanged)
             , this
             , &MSSpectraWnd::handleCurrentChanged );
}


void
MSSpectraWnd::handleSessionAdded( Dataprocessor * processor )
{
    if ( MainWindow::instance()->curPage() != MainWindow::idSelSpectra )
        return;

    if ( auto folder = processor->portfolio().findFolder( L"Spectra" ) ) {
        for ( auto& folium: folder.folio() ) {

            if ( folium.attribute( L"isChecked" ) == L"true" ) {

                portfolio::Folio atts = folium.attachments();
                auto itCentroid = std::find_if( atts.begin(), atts.end(), [] ( const portfolio::Folium& f ){ return f.name() == Constants::F_CENTROID_SPECTRUM; } );

                if ( itCentroid != atts.end() ) {
                    if ( auto centroid = portfolio::get< adcontrols::MassSpectrumPtr >( *itCentroid ) ) {
                        auto it = impl_->dataIds_.find( folium.id() );
                        int idx = (it != impl_->dataIds_.end()) ? std::get<0>( it->second ) : int( impl_->dataIds_.size() + 1 ); // 1.. (0 is reserved for profile overlay data)
                        std::wstring name = processor->file().filename() + L"::" + folium.name();
                        impl_->dataIds_[ folium.id() ] = std::make_tuple( idx, itCentroid->id(), centroid, name );
                        impl_->dirty_ = true;
                    }
                }
            }
        }
    }

    if ( MainWindow::instance()->curPage() != MainWindow::idSelSpectra )
        return;

    if ( impl_->dirty_ ) {
        update_quantable();
        draw();
        impl_->dirty_ = false;
    }
}

void
MSSpectraWnd::handleSelectionChanged( Dataprocessor * processor, portfolio::Folium& folium )
{
    try {
        if ( auto ptr = portfolio::get< adcontrols::MassSpectrumPtr >( folium ) ) {
            impl_->profile_ = std::make_pair( folium.id(), ptr );
        } else {
            return;
        }
    } catch ( ... ) {
        ADERROR() << boost::current_exception_diagnostic_information();
        return;
    }

    bool modified = false;

    if ( folium.attribute( L"isChecked" ) == L"false" ) {

        if ( auto qpks = dataproc_document::instance()->msQuanTable() )
            modified = qpks->erase( folium.id() );

        auto it = impl_->dataIds_.find( folium.id() );
        if ( it != impl_->dataIds_.end() )
            impl_->dataIds_.erase( it );
        
    } else {

        portfolio::Folio atts = folium.attachments();

        auto itCentroid = std::find_if( atts.begin(), atts.end(), [] ( const portfolio::Folium& f ){ return f.name() == Constants::F_CENTROID_SPECTRUM; } );

        if ( itCentroid != atts.end() ) {

            if ( auto centroid = portfolio::get< adcontrols::MassSpectrumPtr >( *itCentroid ) ) {

                auto it = impl_->dataIds_.find( folium.id() );
                int idx = ( it != impl_->dataIds_.end() ) ? std::get<0>( it->second ) : int( impl_->dataIds_.size() + 1 ); // 1.. (0 is reserved for profile overlay data)
                std::wstring name = processor->file().filename() + L"::" + folium.name();

                impl_->dataIds_[ folium.id() ] = std::make_tuple( idx, itCentroid->id(), centroid, name );

                modified = true;
            }

        }
    }

    impl_->dirty_ |= modified; // don't drop previous state if already 'dirty'

    if ( MainWindow::instance()->curPage() != MainWindow::idSelSpectra )
        return;
    
    if ( impl_->dirty_ ) {
        update_quantable();
        draw();
        impl_->dirty_ = false;
    }
}

void
MSSpectraWnd::update_quantable()
{
    if ( auto qpks = dataproc_document::instance()->msQuanTable() ) {

        qpks->clear(); // all clear

        for ( auto& data: impl_->dataIds_ ) {
            const std::wstring& profGuid = data.first;
            const std::wstring& centGuid = std::get<1>(data.second);
            const std::wstring& dataName = std::get<3>(data.second);
            if ( auto centroid = std::get<2>(data.second).lock() )
                qpks->setData( *centroid, centGuid, profGuid, dataName );
        }

    }
}

void
MSSpectraWnd::draw()
{
    if ( auto qpks = dataproc_document::instance()->msQuanTable() ) {
        impl_->table_->setData( qpks );

        if ( auto profile = impl_->profile_.second.lock() ) {
            impl_->plots_[ 1 ]->setData( profile, 0 );
        }

        for ( auto& data: impl_->dataIds_ ) {
            int idx = std::get<0>(data.second);
            if ( auto centroid = std::get<2>(data.second).lock() ) 
                impl_->plots_[ 1 ]->setData( centroid, idx );
        }
    }
}

void
MSSpectraWnd::handleApplyMethod( const adcontrols::ProcessMethod& )
{
}

void
MSSpectraWnd::handleAxisChanged( int axis )
{
    impl_->isTimeAxis_ = ( axis == adplot::SpectrumWidget::HorizontalAxisTime );
    for ( auto& plot: impl_->plots_ )
        plot->setAxis( adplot::SpectrumWidget::HorizontalAxis( axis ), true );
}

void
MSSpectraWnd::handleCheckStateChanged( Dataprocessor *, portfolio::Folium&, bool )
{
    // check state on navigator
    // will handle at handleSelectionChanged
}

void
MSSpectraWnd::handleCurrentChanged( const QString& guid, int idx, int fcn )
{
    // current selection on table has changed.

    if ( auto qpks = dataproc_document::instance()->msQuanTable() ) {
        std::wstring dataGuid = guid.toStdWString();
        std::wstring profGuid = qpks->parentGuid( dataGuid );
        if ( profGuid.empty() )
            return;

        impl_->plots_[1]->setFocusedFcn( fcn );

        auto it = impl_->dataIds_.find( profGuid );
        if ( it != impl_->dataIds_.end() ) {
            // verify guid
            if ( std::get<1>( it->second ) != dataGuid ) {
                ADERROR() << "GUID mismatch -- it is a bug";
                return;
            }
            if ( auto processed = std::get<2>( it->second ).lock() ) {
                adcontrols::segment_wrapper< const adcontrols::MassSpectrum > segs( *processed );
                if ( segs.size() > size_t( fcn ) ) {
                    impl_->markers_[ 1 ]->setPeak( segs[ fcn ], idx, impl_->isTimeAxis_ );
                    impl_->plots_[ 1 ]->replot();
                }
            }
        }
    }
}

void
MSSpectraWnd::onPageSelected()
{
    if ( impl_->dirty_ ) {
        update_quantable();
        draw();
        impl_->dirty_ = false;
    }
}

void
MSSpectraWnd::handleProcessed( Dataprocessor * processor, portfolio::Folium& folium )
{
    handleSelectionChanged( processor, folium );
}

void
MSSpectraWnd::onDataChanged( const QString& foliumGuid, const QString& attGuid, int idx, int fcn )
{
    // data changed on MSPeakTable via MSProcessingWnd
    (void)fcn;
    (void)idx;
    (void)attGuid;
    auto it = impl_->dataIds_.find( foliumGuid.toStdWString() );
    if ( it != impl_->dataIds_.end() ) {

        // pointer for spectrum is weak share with the portfolio, so centroid spectrum should be up to date
        // without pull data out again from portfolio

        if ( auto centroid = std::get<2>(it->second).lock() ) {
            if ( auto qpks = dataproc_document::instance()->msQuanTable() ) {

                qpks->erase( foliumGuid.toStdWString() );

                const std::wstring& profGuid = it->first;
                const std::wstring& centGuid = std::get<1>(it->second);
                const std::wstring& dataName = std::get<3>(it->second);
                qpks->setData( *centroid, centGuid, profGuid, dataName );

                impl_->dirty_ = true;
            }
        }
    }

    if ( impl_->dirty_ && ( MainWindow::instance()->curPage() == MainWindow::idSelSpectra ) ) {
        update_quantable();
        draw();
        impl_->dirty_ = false;
    }
}

void
MSSpectraWnd::handleDataChanged( const QString& dataGuid, int idx, int fcn, int column, const QVariant& data )
{
    (void)data;
    (void)column;
    (void)fcn;
    (void)idx;
    // data changed on MSQuanTable
    if ( auto qpks = dataproc_document::instance()->msQuanTable() ) {
        const std::wstring& guid = qpks->parentGuid( dataGuid.toStdWString() );
        if ( guid.empty() )
            return;
    }
}

///////////////////////////

