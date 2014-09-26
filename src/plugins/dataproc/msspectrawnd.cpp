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
#include <adplugin/widget_factory.hpp>
#include <adportable/configuration.hpp>
#include <adplot/chromatogramwidget.hpp>
#include <adplot/spectrumwidget.hpp>
#include <adplot/spectrogramwidget.hpp>
#include <adplot/peakmarker.hpp>
#include <adwidgets/msquantable.hpp>
#include <qtwidgets/peakresultwidget.hpp>

#include <portfolio/folium.hpp>
#include <portfolio/folder.hpp>
#include <portfolio/portfolio.hpp>
#include <boost/variant.hpp>
#include <boost/any.hpp>
#include <boost/exception/all.hpp>
#include <qtwrapper/qstring.hpp>
#include <coreplugin/minisplitter.h>
#include <QBoxLayout>

using namespace dataproc;

MSSpectraWnd::~MSSpectraWnd()
{
}

MSSpectraWnd::MSSpectraWnd( QWidget *parent ) :  QWidget(parent)
                                              , plot_( new adplot::SpectrumWidget )
                                              , table_( new adwidgets::MSQuanTable )
                                              , marker_( new adplot::PeakMarker )
                                              , isTimeAxis_( false )
                                              , dirty_( false )
{
    init();
}

void
MSSpectraWnd::init()
{
    Core::MiniSplitter * splitter = new Core::MiniSplitter;
    if ( splitter ) {
        if ( adplugin::LifeCycle * p = dynamic_cast<adplugin::LifeCycle *>(table_.get()) ) {
            p->OnInitialUpdate();
        }

        connect( plot_.get()
                 , static_cast< void(adplot::SpectrumWidget::*)(const QRectF&)>(&adplot::SpectrumWidget::onSelected)
                 , [=]( const QRectF& rc){ table_->handleSelected( rc, isTimeAxis_); });

        plot_->enableAxis( QwtPlot::yRight );
        marker_->attach( plot_.get() );
        marker_->visible( true );
        marker_->setYAxis( QwtPlot::yRight );

        splitter->addWidget( plot_.get() );
        splitter->addWidget( table_.get() );
        splitter->setOrientation( Qt::Vertical );
    }

    QBoxLayout * toolBarAddingLayout = new QVBoxLayout( this );
    toolBarAddingLayout->setMargin(0);
    toolBarAddingLayout->setSpacing(0);
    toolBarAddingLayout->addWidget( splitter );

    connect( table_.get()
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
                        auto it = dataIds_.find( folium.id() );
                        int idx = (it != dataIds_.end()) ? std::get<0>( it->second ) : int( dataIds_.size() + 1 ); // 1.. (0 is reserved for profile overlay data)
                        std::wstring name = processor->file().filename() + L"::" + folium.name();
                        dataIds_[ folium.id() ] = std::make_tuple( idx, itCentroid->id(), centroid, name );
                        dirty_ = true;
                    }
                }
            }
        }
    }

    if ( MainWindow::instance()->curPage() != MainWindow::idSelSpectra )
        return;

    if ( dirty_ ) {
        update_quantable();
        draw();
        dirty_ = false;
    }
}

void
MSSpectraWnd::handleSelectionChanged( Dataprocessor * processor, portfolio::Folium& folium )
{
    try {
        if ( auto ptr = portfolio::get< adcontrols::MassSpectrumPtr >( folium ) ) {
            profile_ = std::make_pair( folium.id(), ptr );
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

        auto it = dataIds_.find( folium.id() );
        if ( it != dataIds_.end() )
            dataIds_.erase( it );
        
    } else {

        portfolio::Folio atts = folium.attachments();

        auto itCentroid = std::find_if( atts.begin(), atts.end(), [] ( const portfolio::Folium& f ){ return f.name() == Constants::F_CENTROID_SPECTRUM; } );

        if ( itCentroid != atts.end() ) {

            if ( auto centroid = portfolio::get< adcontrols::MassSpectrumPtr >( *itCentroid ) ) {

                auto it = dataIds_.find( folium.id() );
                int idx = (it != dataIds_.end()) ? std::get<0>( it->second ) : int( dataIds_.size() + 1 ); // 1.. (0 is reserved for profile overlay data)
                std::wstring name = processor->file().filename() + L"::" + folium.name();

                dataIds_[ folium.id() ] = std::make_tuple( idx, itCentroid->id(), centroid, name );

                modified = true;
            }

        }
    }

    dirty_ |= modified; // don't drop previous state if already 'dirty'

    if ( MainWindow::instance()->curPage() != MainWindow::idSelSpectra )
        return;
    
    if ( dirty_ ) {
        update_quantable();
        draw();
        dirty_ = false;
    }
}

void
MSSpectraWnd::update_quantable()
{
    if ( auto qpks = dataproc_document::instance()->msQuanTable() ) {

        qpks->clear(); // all clear

        for ( auto& data: dataIds_ ) {
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
        table_->setData( qpks );

        if ( auto profile = profile_.second.lock() ) {
            plot_->setData( profile, 0 );
        }

        for ( auto& data: dataIds_ ) {
            int idx = std::get<0>(data.second);
            if ( auto centroid = std::get<2>(data.second).lock() ) 
                plot_->setData( centroid, idx );
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
    isTimeAxis_ = ( axis == adplot::SpectrumWidget::HorizontalAxisTime );
    plot_->setAxis( adplot::SpectrumWidget::HorizontalAxis( axis ), true );
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

        plot_->setFocusedFcn( fcn );

        auto it = dataIds_.find( profGuid );
        if ( it != dataIds_.end() ) {
            // verify guid
            if ( std::get<1>( it->second ) != dataGuid ) {
                ADERROR() << "GUID mismatch -- it is a bug";
                return;
            }
            if ( auto processed = std::get<2>( it->second ).lock() ) {
                adcontrols::segment_wrapper< const adcontrols::MassSpectrum > segs( *processed );
                if ( segs.size() > size_t( fcn ) ) {
                    marker_->setPeak( segs[ fcn ], idx, isTimeAxis_ );
                    plot_->replot();
                }
            }
        }
    }
}

void
MSSpectraWnd::onPageSelected()
{
    if ( dirty_ ) {
        update_quantable();
        draw();
        dirty_ = false;
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

    auto it = dataIds_.find( foliumGuid.toStdWString() );
    if ( it != dataIds_.end() ) {

        // pointer for spectrum is weak share with the portfolio, so centroid spectrum should be up to date
        // without pull data out again from portfolio

        if ( auto centroid = std::get<2>(it->second).lock() ) {
            if ( auto qpks = dataproc_document::instance()->msQuanTable() ) {

                qpks->erase( foliumGuid.toStdWString() );

                const std::wstring& profGuid = it->first;
                const std::wstring& centGuid = std::get<1>(it->second);
                const std::wstring& dataName = std::get<3>(it->second);
                qpks->setData( *centroid, centGuid, profGuid, dataName );

                dirty_ = true;
            }
        }
    }

    if ( dirty_ && ( MainWindow::instance()->curPage() == MainWindow::idSelSpectra ) ) {
        update_quantable();
        draw();
        dirty_ = false;
    }
}

void
MSSpectraWnd::handleDataChanged( const QString& dataGuid, int idx, int fcn, int column, const QVariant& data )
{
    // data changed on MSQuanTable
    if ( auto qpks = dataproc_document::instance()->msQuanTable() ) {
        const std::wstring& guid = qpks->parentGuid( dataGuid.toStdWString() );
        if ( guid.empty() )
            return;
    }
}

///////////////////////////

