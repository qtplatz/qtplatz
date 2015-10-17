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

    struct datafolder {
        int idx;
        std::wstring display_name; // fileneme::folium.name
        std::wstring idFolium;
        std::wstring idCentroid;
        std::weak_ptr< adcontrols::MassSpectrum > profile;    // usually profile, TBD for histogram data
        std::weak_ptr< adcontrols::MassSpectrum > centroid;  // centroid

        datafolder( int _0 = 0
                    , const std::wstring& _1 = std::wstring()
                    , const std::wstring& _2 = std::wstring()
                    , const std::wstring& _3 = std::wstring() ) : idx( _0 )
                                                                , display_name( _1 )
                                                                , idFolium( _2 )
                                                                , idCentroid( _3 ) {
            
        }

        datafolder( int _idx
                    , const std::wstring& _display_name
                    , portfolio::Folium& folium ) : idx( _idx )
                                                   , display_name( _display_name )
                                                   , idFolium( folium.id() )  {
            
            if ( auto ms = portfolio::get< adcontrols::MassSpectrumPtr >( folium ) ) {
                profile = ms; // maybe profile or histogram
            }

            portfolio::Folio atts = folium.attachments();
            auto itCentroid = std::find_if( atts.begin(), atts.end(), [] ( const portfolio::Folium& f ){ return f.name() == Constants::F_CENTROID_SPECTRUM; } );
            if ( itCentroid != atts.end() ) {
                
                idCentroid = itCentroid->id();
                centroid = portfolio::get< adcontrols::MassSpectrumPtr >( *itCentroid );
                
            }
        }

        datafolder( const datafolder& t ) : idx( t.idx )
                                          , idFolium( t.idFolium )
                                          , idCentroid( t.idCentroid )
                                          , display_name( t.display_name )
                                          , profile( t.profile )
                                          , centroid( t.centroid ) {
        }

    };

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

        std::map< std::wstring /* folium (profile) Guid (attGuid) */, datafolder  > dataIds_;

        std::pair< std::wstring, datafolder > profile_;

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
            plot->setMinimumHeight( 80 );
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
                
                if ( auto profile = portfolio::get< adcontrols::MassSpectrumPtr >( folium ) ) {

                    std::wstring display_name = processor->file().filename() + L"::" + folium.name();

                    auto it = impl_->dataIds_.find( folium.id() );
                    if ( it == impl_->dataIds_.end() ) {
                        
                        auto data = datafolder( int( impl_->dataIds_.size() ), display_name, folium );
                        impl_->dataIds_[ folium.id() ] = data;

                    }
                    
                }
            }
        }
    }

    if ( MainWindow::instance()->curPage() != MainWindow::idSelSpectra )
        return;

    if ( impl_->dirty_ ) {
        update_quantable();
        draw( -1 );
        impl_->dirty_ = false;
    }
}

void
MSSpectraWnd::handleSelectionChanged( Dataprocessor * processor, portfolio::Folium& folium )
{
    try {
        if ( auto ptr = portfolio::get< adcontrols::MassSpectrumPtr >( folium ) ) {
            std::wstring display_name = processor->file().filename() + L"::" + folium.name();
            impl_->profile_ = std::make_pair( folium.id(), datafolder( 0, display_name, folium ) );
            draw( 1 );
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

        // TODO:  add to spectrumwidget[1]
        
    } else {

        if ( auto profile = portfolio::get< adcontrols::MassSpectrumPtr >( folium ) ) {

            std::wstring display_name = processor->file().filename() + L"::" + folium.name();
            
            auto it = impl_->dataIds_.find( folium.id() );
            if ( it == impl_->dataIds_.end() ) {
                
                auto data = datafolder( int( impl_->dataIds_.size() ), display_name, folium.id() );
                data.profile = profile;
                
                portfolio::Folio atts = folium.attachments();
                auto itCentroid = std::find_if( atts.begin(), atts.end(), [] ( const portfolio::Folium& f ){ return f.name() == Constants::F_CENTROID_SPECTRUM; } );
                if ( itCentroid != atts.end() ) {
                    
                    data.idCentroid = itCentroid->id();
                    data.centroid = portfolio::get< adcontrols::MassSpectrumPtr >( *itCentroid );
                    
                }
                
                impl_->dataIds_[ folium.id() ] = data;

                modified = true;
                
            }
        }
    }

    impl_->dirty_ |= modified; // don't drop previous state if already 'dirty'

    if ( MainWindow::instance()->curPage() != MainWindow::idSelSpectra )
        return;
    
    if ( impl_->dirty_ ) {
        update_quantable();
        draw( 0 );
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
            const std::wstring& centGuid = data.second.idCentroid;// std::get<1>( data.second );
            const std::wstring& dataName = data.second.display_name; // std::get<3>( data.second );

            if ( auto centroid = data.second.centroid.lock() ) //std::get<2>(data.second).lock() )
                qpks->setData( *centroid, centGuid, profGuid, dataName );
        }

    }
}

void
MSSpectraWnd::draw( int which )
{
    if ( which == 1 || which == ( -1 ) ) {
        impl_->plots_[ 1 ]->clear();
        impl_->plots_[ 1 ]->setTitle( impl_->profile_.second.display_name );
        if ( auto ms = impl_->profile_.second.profile.lock() ) {
            impl_->plots_[ 1 ]->setData( ms, 0 );
        }
        if ( auto ms = impl_->profile_.second.centroid.lock() ) {
            impl_->plots_[ 1 ]->setData( ms, 1, true );
            impl_->plots_[ 1 ]->setAlpha( 0, 0x40 ); 
        }
    }

    if ( which == 0 || which == ( -1 ) ) {
        
        if ( auto qpks = dataproc_document::instance()->msQuanTable() ) {
            impl_->table_->setData( qpks );

            impl_->plots_[ 0 ]->clear();

            QString title;

            for ( auto& data: impl_->dataIds_ ) {
                int idx = data.second.idx;

                if ( title.isEmpty() ) {
                    title = QString::fromStdWString( data.second.display_name );
                } else {
                    title += " .";
                }

                QColor color = impl_->plots_[ 0 ]->index_color( idx );

                if ( auto profile = data.second.profile.lock() ) {
                    impl_->plots_[ 0 ]->setData( profile, idx * 2 );
                    impl_->plots_[ 0 ]->setColor( idx * 2, color );
                }
                if ( auto centroid = data.second.centroid.lock() ) {
                    impl_->plots_[ 0 ]->setData( centroid, idx * 2 + 1, true );
                    impl_->plots_[ 0 ]->setColor( idx * 2 + 1, color );
                    impl_->plots_[ 0 ]->setAlpha( idx * 2, 0x40 ); 
                }
                
            }
            impl_->plots_[ 0 ]->setTitle( title );
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
            if ( it->second.idCentroid != dataGuid ) {
                ADERROR() << "GUID mismatch -- it is a bug";
                return;
            }
            if ( auto processed = it->second.centroid.lock() ) { //std::get<2>( it->second ).lock() ) {
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

        if ( auto centroid = it->second.centroid.lock() ) { //std::get<2>(it->second).lock() ) {
            if ( auto qpks = dataproc_document::instance()->msQuanTable() ) {

                qpks->erase( foliumGuid.toStdWString() );

                const std::wstring& profGuid = it->first;
                const std::wstring& centGuid = it->second.idCentroid; // std::get<1>( it->second );
                const std::wstring& dataName = it->second.display_name; // std::get<3>( it->second );
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

