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
#include "document.hpp"
#include <adcontrols/description.hpp>
#include <adcontrols/datafile.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msqpeaks.hpp>
#include <adutils/processeddata.hpp>
#include <adwplot/spectrogramwidget.hpp>
#include <adwplot/peakmarker.hpp>
#include <adwidgets/msquantable.hpp>
#include <portfolio/folium.hpp>
#include <portfolio/folder.hpp>
#include <portfolio/portfolio.hpp>
#include <boost/variant.hpp>
#include <boost/any.hpp>

#include <qtwrapper/qstring.hpp>
#include <coreplugin/minisplitter.h>
#include <QBoxLayout>
#include <adportable/configuration.hpp>

#include <adplugin/lifecycle.hpp>
#include <adplugin/plugin.hpp>
#include <adplugin/plugin_ptr.hpp>
#include <adplugin/widget_factory.hpp>

#include <qtwidgets/peakresultwidget.hpp>
#include <adwplot/chromatogramwidget.hpp>
#include <adwplot/spectrumwidget.hpp>
#include "qtwidgets_name.hpp"

using namespace dataproc;

MSSpectraWnd::~MSSpectraWnd()
{
}

MSSpectraWnd::MSSpectraWnd( QWidget *parent ) :  QWidget(parent)
                                              , plot_( new adwplot::SpectrumWidget )
                                              , table_( new adwidgets::MSQuanTable )
                                              , marker_( new adwplot::PeakMarker )
                                              , isTimeAxis_( false )
                                              , dirty_( true )
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
        // connect( pImpl_->profileSpectrum_, SIGNAL( onSelected( const QRectF& ) ), this, SLOT( selectedOnProfile( const QRectF& ) ) );
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
        , static_cast<void (adwidgets::MSQuanTable::*)(int, int, const QString&, const QString&)>(&adwidgets::MSQuanTable::currentChanged)
        , this
        , &MSSpectraWnd::handleCurrentChanged );
}

void
MSSpectraWnd::handleSessionAdded( Dataprocessor * processor )
{
    if ( MainWindow::instance()->curPage() != MainWindow::idSelSpectra )
        return;

    auto * qpks = document::instance()->msQuanTable();

    int idx = 0;
    if ( auto folder = processor->portfolio().findFolder( L"Spectra" ) ) {

        for ( auto& folium: folder.folio() ) {
            if ( folium.attribute( L"isChecked" ) == L"true" ) {
                if ( folium.empty() )
                    processor->fetch( folium );
                if ( idx == 0 ) {
                    auto profile = portfolio::get< adcontrols::MassSpectrumPtr >( folium );
                    plot_->setData( profile, idx++, true );
                }
                auto atts = folium.attachments();
                auto itCentroid = std::find_if( atts.begin(), atts.end(), []( const portfolio::Folium& f ){ return f.name() == Constants::F_CENTROID_SPECTRUM; });
                if ( itCentroid != atts.end() ) {
                    if ( auto centroid = portfolio::get< adcontrols::MassSpectrumPtr >( *itCentroid ) ) {
                        dataIds_[ folium.id() ] = std::make_tuple( idx, itCentroid->id(), centroid );
                        plot_->setData( centroid, idx++ );
                        qpks->setData( *centroid, itCentroid->id(), folium.id(), processor->file().filename() + L"::" + folium.name() );
                        table_->setData( qpks );
                    }
                }
            }
        }
    }
}

void
MSSpectraWnd::handleProcessed( Dataprocessor * processor, portfolio::Folium& folium )
{
    handleSelectionChanged( processor, folium );
}

void
MSSpectraWnd::handleSelectionChanged( Dataprocessor * processor, portfolio::Folium& folium )
{
    if ( MainWindow::instance()->curPage() != MainWindow::idSelSpectra )
        return;

    if ( auto folder = folium.getParentFolder() ) {
        if ( folder.name() != L"Spectra" )
            return;
    }

    if ( folium.attribute( L"isChecked" ) == L"false" ) {

        auto qpks = document::instance()->msQuanTable();
        if ( qpks->erase( folium.id() ) )
            table_->setData( qpks );

        auto it = dataIds_.find( folium.id() );
        if ( it != dataIds_.end() ) {
            plot_->removeData( std::get<0>(it->second) );  // 0 := index, 1 := guid, 2 := weak_ptr<MassSpectrum>
            dataIds_.erase( it );
        }
        return;
    }

    if ( auto profile = portfolio::get< adcontrols::MassSpectrumPtr >( folium ) ) {
        plot_->setData( profile, 0, true );
        profile_ = profile;
    }

    int idx = int( dataIds_.size() + 1 ); // start with 1 ( 0 was reserved for profile )
    auto it = dataIds_.find( folium.id() );
    if ( it != dataIds_.end() )
        idx = std::get<0>( it->second );

    auto atts = folium.attachments();
    auto itCentroid = std::find_if( atts.begin(), atts.end(), []( const portfolio::Folium& f ){ return f.name() == Constants::F_CENTROID_SPECTRUM; });
    if ( itCentroid != atts.end() ) {
        if ( auto centroid = portfolio::get< adcontrols::MassSpectrumPtr >( *itCentroid ) ) {
            dataIds_[ folium.id() ] = std::make_tuple( idx, itCentroid->id(), centroid );
            plot_->setData( centroid, idx );
            if ( auto * qpks = document::instance()->msQuanTable() ) {
                qpks->setData( *centroid, itCentroid->id(), folium.id(), processor->file().filename() + L"::" + folium.name() );
                table_->setData( qpks );
            }
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
    isTimeAxis_ = ( axis == adwplot::SpectrumWidget::HorizontalAxisTime );
    plot_->setAxis( adwplot::SpectrumWidget::HorizontalAxis( axis ), true );
}

void
MSSpectraWnd::handleCheckStateChanged( Dataprocessor *, portfolio::Folium&, bool )
{
    // will handle at handleSelectionChanged
}

void
MSSpectraWnd::handleCurrentChanged( int idx, int fcn, const QString& dataGuid, const QString& parentGuid )
{
    std::wstring profGuid = parentGuid.toStdWString();
    std::wstring centGuid = dataGuid.toStdWString();

    plot_->setFocusedFcn( fcn );
    auto it = dataIds_.find( profGuid );
    if ( it != dataIds_.end() ) {
        if ( std::get<1>( it->second ) != centGuid ) {
            ADDEBUG() << "GUID mismatch -- it is a bug";
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

void
MSSpectraWnd::onPageSelected()
{
    
}

///////////////////////////

