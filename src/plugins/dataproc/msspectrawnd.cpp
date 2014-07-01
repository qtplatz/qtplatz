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
            //connect( this, SIGNAL( fireSetData( const adcontrols::PeakResult& ) ),
            //    pImpl_->peakWidget_, SLOT( setData( const adcontrols::PeakResult& ) ) );
        }
        splitter->addWidget( plot_.get() );
        splitter->addWidget( table_.get() );
        splitter->setOrientation( Qt::Vertical );
    }

    QBoxLayout * toolBarAddingLayout = new QVBoxLayout( this );
    toolBarAddingLayout->setMargin(0);
    toolBarAddingLayout->setSpacing(0);
    toolBarAddingLayout->addWidget( splitter );
}

void
MSSpectraWnd::handleSessionAdded( Dataprocessor * processor )
{
    dataIds_.clear();
    auto * qpks = document::instance()->msQuanTable();
    qpks->clear();

    int idx = 0;
    if ( auto folder = processor->portfolio().findFolder( L"Spectra" ) ) {

		// fullpath_ = QString::fromStdWString( processor->filename() );
        for ( auto& folium: folder.folio() ) {
            if ( folium.attribute( L"isChecked" ) == L"true" ) {
                if ( folium.empty() )
                    processor->fetch( folium );
                auto atts = folium.attachments();
                auto itCentroid = std::find_if( atts.begin(), atts.end(), []( const portfolio::Folium& f ){ return f.name() == Constants::F_CENTROID_SPECTRUM; });
                if ( itCentroid != atts.end() ) {
                    dataIds_[ folium.id() ] = idx; // keep id for profile (quicker to find than an attachment id)
                    auto centroid = portfolio::get< adcontrols::MassSpectrumPtr >( *itCentroid );
                    plot_->setData( centroid, idx++ );
                    qpks->setData( *centroid, itCentroid->id(), folium.id(), processor->file().filename() + L"::" + folium.name() );
                    table_->setData( qpks );
                }
            }
        }
    }
}

void
MSSpectraWnd::handleSelectionChanged( Dataprocessor * processor, portfolio::Folium& folium )
{
    if ( folium.attribute( L"isChecked" ) == L"false" ) {
        auto qpks = document::instance()->msQuanTable();
        qpks->erase( folium.id() );
        table_->setData( qpks );

        auto it = dataIds_.find( folium.id() );
        if ( it != dataIds_.end() ) {
            plot_->removeData( it->second );
            dataIds_.erase( it );
        }
        return;
    }


    int idx = 0;
    auto it = dataIds_.find( folium.id() );
    if ( it != dataIds_.end() )
        idx = it->second;
    else
        idx = int( dataIds_.size() );
    auto atts = folium.attachments();
    auto itCentroid = std::find_if( atts.begin(), atts.end(), []( const portfolio::Folium& f ){ return f.name() == Constants::F_CENTROID_SPECTRUM; });
    if ( itCentroid != atts.end() ) {
        auto centroid = portfolio::get< adcontrols::MassSpectrumPtr >( *itCentroid );
        dataIds_[ folium.id() ] = idx;
        plot_->setData( centroid, idx );
        if ( auto * qpks = document::instance()->msQuanTable() ) {
            qpks->setData( *centroid, itCentroid->id(), folium.id(), processor->file().filename() + L"::" + folium.name() );
            table_->setData( qpks );
        }
    }
}

void
MSSpectraWnd::handleApplyMethod( const adcontrols::ProcessMethod& )
{
}

void
MSSpectraWnd::handleAxisChanged( int )
{
}

void
MSSpectraWnd::handleCheckStateChanged( Dataprocessor *, portfolio::Folium&, bool )
{
}
///////////////////////////

// void
// MSSpectraWndImpl::setData( const adcontrols::Chromatogram& c, const QString& )
// {
//     chroWidget_->setData( c );
// }

