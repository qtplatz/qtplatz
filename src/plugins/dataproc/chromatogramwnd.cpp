// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include "chromatogramwnd.hpp"

#if ! defined Q_MOC_RUN
#include "dataprocessor.hpp"
#include "dataprocconstants.hpp"
#include "selchanged.hpp"
#include "qtwidgets_name.hpp"
#include <adcontrols/description.hpp>
#include <adcontrols/datafile.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/peakresult.hpp>
#include <adcontrols/peaks.hpp>
#include <adcontrols/peak.hpp>
#include <adplugin/lifecycle.hpp>
#include <adplugin/plugin.hpp>
#include <adplugin/plugin_ptr.hpp>
//#include <adplugin/widget_factory.hpp>
#include <adplot/chromatogramwidget.hpp>
#include <adplot/spectrumwidget.hpp>
#include <adplot/peakmarker.hpp>
#include <adportable/configuration.hpp>
#include <adutils/processeddata.hpp>
#include <adwidgets/peaktable.hpp>
#include <portfolio/folium.hpp>
#include <qtwrapper/qstring.hpp>
#include <boost/variant.hpp>
#include <boost/any.hpp>
#endif

#include <coreplugin/minisplitter.h>
#include <QAction>
#include <QBoxLayout>
#include <QShortcut>
#include <QMessageBox>

using namespace dataproc;

namespace dataproc {

    class ChromatogramWnd::impl : public QObject {
        Q_OBJECT
    public:
        ~impl() {
            marker_.reset();
            delete chroWidget_;
            delete peakTable_;
        }
        impl( ChromatogramWnd * p ) : QObject( p )
                                    , this_( p )
                                    , chroWidget_( new adplot::ChromatogramWidget )
                                    , peakTable_(new adwidgets::PeakTable)
                                    , marker_( std::make_shared< adplot::PeakMarker >() ) {

            using adwidgets::PeakTable;

            auto shortcut = new QShortcut( QKeySequence::Copy, p );
            connect( shortcut, &QShortcut::activatedAmbiguously, this, &impl::copy );
            connect( peakTable_, static_cast<void(PeakTable::*)(int)>(&PeakTable::currentChanged), this, &impl::handleCurrentChanged );

            marker_->attach( chroWidget_ );
            marker_->visible( true );
            marker_->setYAxis( QwtPlot::yLeft );
            
        }

        void setData( adcontrols::ChromatogramPtr& ptr ) {
            data_ = ptr;
            chroWidget_->setData( ptr );
            peakResult_.reset();
            if ( ptr->peaks().size() )
                peakResult_ = std::make_shared< adcontrols::PeakResult >( ptr->baselines(), ptr->peaks() );
        }

        void setData( adcontrols::PeakResultPtr& ptr ) {
            peakResult_ = ptr;
            chroWidget_->setData( *ptr );
            peakTable_->setData( *ptr );
        }

        void handleCurrentChanged( int peakId ) {
            using adcontrols::Peak;
            if ( peakResult_ ) {
                const auto& peaks = peakResult_->peaks();
                auto it = std::find_if( peaks.begin(), peaks.end(), [peakId] ( const Peak& pk ){
                        return pk.peakId() == peakId;
                    } );
                if ( it != peaks.end() ) {
                    marker_->setPeak( *it );
                    chroWidget_->replot();
                }
            }
        }

        ChromatogramWnd * this_;
        adplot::ChromatogramWidget * chroWidget_;
        adwidgets::PeakTable * peakTable_;
        std::shared_ptr< adplot::PeakMarker > marker_;
        adcontrols::ChromatogramPtr data_;
        adcontrols::PeakResultPtr peakResult_;
    public slots:
        void copy() {
            peakTable_->copy();
        }
    };

    //----------------------------//
    template<class Wnd> struct selProcessed : public boost::static_visitor<void> {
        selProcessed( Wnd& wnd ) : wnd_(wnd) {}
        template<typename T> void operator ()( T& ) const {
        }
        void operator () ( adutils::MassSpectrumPtr& ptr ) const {   
            wnd_.draw2( ptr );
        }
        void operator () ( adutils::ChromatogramPtr& ptr ) const {
            wnd_.draw( ptr );
        }
        void operator () ( adutils::PeakResultPtr& ptr ) const {
            wnd_.draw( ptr );
        }
        Wnd& wnd_;
    };

}

ChromatogramWnd::~ChromatogramWnd()
{
}

ChromatogramWnd::ChromatogramWnd( QWidget *parent ) : QWidget(parent)
                                                    , impl_( new impl( this ) )
{
    Core::MiniSplitter * splitter = new Core::MiniSplitter;

    if ( splitter ) {

        splitter->addWidget( impl_->chroWidget_ );
        splitter->addWidget( impl_->peakTable_ );
        splitter->setOrientation( Qt::Vertical );
    }

    QBoxLayout * toolBarAddingLayout = new QVBoxLayout( this );
    toolBarAddingLayout->setMargin(0);
    toolBarAddingLayout->setSpacing(0);
    toolBarAddingLayout->addWidget( splitter );

    impl_->peakTable_->OnInitialUpdate();
}

void
ChromatogramWnd::draw1( adutils::MassSpectrumPtr& )
{
}

void
ChromatogramWnd::draw2( adutils::MassSpectrumPtr& )
{
}

void
ChromatogramWnd::draw( adutils::ChromatogramPtr& ptr )
{
    impl_->setData( ptr );
    // impl_->chroWidget_->setData( ptr );
    // impl_->peakTable_->setData( adcontrols::PeakResult( ptr->baselines(), ptr->peaks() ) );
}

void
ChromatogramWnd::draw( adutils::PeakResultPtr& ptr )
{
    impl_->setData( ptr );
	// impl_->chroWidget_->setData( *ptr );
    // impl_->peakTable_->setData( *ptr );
}

void
ChromatogramWnd::handleSessionAdded( Dataprocessor * )
{
	/*
    adcontrols::datafile& file = processor->file();
    QString filename( qtwrapper::qstring::copy( file.filename() ) );
    const adcontrols::LCMSDataset * dset = processor->getLCMSDataset();
    if ( dset ) {
        adcontrols::Chromatogram c;
        if ( dset->getTIC( 0, c ) ) {
            c.addDescription( adcontrols::Description( L"filename", file.filename() ) );
            //impl_->setData( c, filename );
        }
    }
	*/
}

void
ChromatogramWnd::handleProcessed( Dataprocessor* , portfolio::Folium& folium )
{
    adutils::ProcessedData::value_type data = adutils::ProcessedData::toVariant( static_cast<boost::any&>( folium ) );
    boost::apply_visitor( selChanged<ChromatogramWnd>(*this), data );

    portfolio::Folio attachments = folium.attachments();
    for ( portfolio::Folio::iterator it = attachments.begin(); it != attachments.end(); ++it ) {
        adutils::ProcessedData::value_type contents = adutils::ProcessedData::toVariant( static_cast<boost::any&>( *it ) );
        boost::apply_visitor( selProcessed<ChromatogramWnd>( *this ), contents );
    }
}

void
ChromatogramWnd::handleSelectionChanged( Dataprocessor* , portfolio::Folium& folium )
{
    adutils::ProcessedData::value_type data = adutils::ProcessedData::toVariant( static_cast<boost::any&>( folium ) );
    boost::apply_visitor( selChanged<ChromatogramWnd>(*this), data );

    portfolio::Folio attachments = folium.attachments();
    for ( portfolio::Folio::iterator it = attachments.begin(); it != attachments.end(); ++it ) {
        adutils::ProcessedData::value_type contents = adutils::ProcessedData::toVariant( static_cast<boost::any&>( *it ) );
        boost::apply_visitor( selProcessed<ChromatogramWnd>( *this ), contents );
    }
}

void
ChromatogramWnd::handleApplyMethod( const adcontrols::ProcessMethod& )
{
}

///////////////////////////

#include "chromatogramwnd.moc"
