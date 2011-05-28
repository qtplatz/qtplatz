// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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
#include "dataprocessor.hpp"
#include "selchanged.hpp"
#include <adcontrols/description.hpp>
#include <adcontrols/datafile.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adutils/processeddata.hpp>
#include <portfolio/folium.hpp>
#include <boost/variant.hpp>
#include <boost/any.hpp>
#include <qtwrapper/qstring.hpp>
#include <coreplugin/minisplitter.h>
#include <QBoxLayout>
#include <adportable/configuration.hpp>
#include <adplugin/adplugin.hpp>
#include <adplugin/lifecycle.hpp>
#include <qtwidgets/peakresultwidget.hpp>
#include <adwplot/chromatogramwidget.hpp>
#include <adwplot/spectrumwidget.hpp>

using namespace dataproc;
using namespace dataproc::internal;

namespace dataproc {
    namespace internal {

        class ChromatogramWndImpl {
        public:
            ~ChromatogramWndImpl() {
                delete chroWidget_;
                delete peakWidget_;
            }
            ChromatogramWndImpl() : chroWidget_(0)
                                  , peakWidget_(0) {
            }
            void setData( const adcontrols::Chromatogram&, const QString& );
            adwplot::ChromatogramWidget * chroWidget_;
            //adwidgets::ui::PeakResultWidget * peakWidget_;
            QWidget * peakWidget_;
        };

        //----------------------------//
        template<class Wnd> struct selProcessed : public boost::static_visitor<void> {
            selProcessed( Wnd& wnd ) : wnd_(wnd) {}
            template<typename T> void operator ()( T& ) const { }
            template<> void operator () ( adutils::MassSpectrumPtr& ptr ) const {   
                wnd_.draw2( ptr );
            }
            template<> void operator () ( adutils::ChromatogramPtr& ptr ) const {
                wnd_.draw( ptr );
            }
            Wnd& wnd_;
        };
        //----------------------------//
    }
}

ChromatogramWnd::~ChromatogramWnd()
{
}

ChromatogramWnd::ChromatogramWnd(const std::wstring& apppath, QWidget *parent) :  QWidget(parent)
{
    init( apppath );
}

void
ChromatogramWnd::init( const std::wstring& apppath )
{
    pImpl_.reset( new ChromatogramWndImpl );
    Core::MiniSplitter * splitter = new Core::MiniSplitter;
    if ( splitter ) {
        if ( pImpl_->chroWidget_ = new adwplot::ChromatogramWidget( this ) ) {

            // peak table
            adportable::Configuration config;
            adportable::Module module;
#if defined _DEBUG
            module.library_filename( L"/lib/qtPlatz/plugins/ScienceLiaison/qtwidgetsd.dll" );
#else
            module.library_filename( L"/lib/qtPlatz/plugins/ScienceLiaison/qtwidgets.dll" );
#endif
            config.module( module );
            config.interface( L"qtwidgets::PeakResultWidget" );

            pImpl_->peakWidget_ = adplugin::manager::widget_factory( config, apppath.c_str() );
            if ( pImpl_->peakWidget_ ) {
                adplugin::LifeCycle * p = dynamic_cast< adplugin::LifeCycle * >(pImpl_->peakWidget_);
                if ( p )
                    p->OnInitialUpdate();
                connect( this, SIGNAL( fireSetData( const adcontrols::Chromatogram& ) ),
                    pImpl_->peakWidget_, SLOT( setData( const adcontrols::Chromatogram& ) ) );
            }

            splitter->addWidget( pImpl_->chroWidget_ );
            splitter->addWidget( pImpl_->peakWidget_ );
            splitter->setOrientation( Qt::Vertical );
        }
    }

    QBoxLayout * toolBarAddingLayout = new QVBoxLayout( this );
    toolBarAddingLayout->setMargin(0);
    toolBarAddingLayout->setSpacing(0);
    //toolBarAddingLayout->addWidget( toolBar );
    toolBarAddingLayout->addWidget( splitter );
    //toolBarAddingLayout->addWidget( toolBar2 );
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
    adcontrols::Chromatogram& c = *ptr;
    pImpl_->chroWidget_->setData( c );
    emit fireSetData( c );
    // pImpl_->peakWidget_->setData( c );
}

void
ChromatogramWnd::handleSessionAdded( Dataprocessor * processor )
{
    adcontrols::datafile& file = processor->file();
    QString filename( qtwrapper::qstring::copy( file.filename() ) );
    const adcontrols::LCMSDataset * dset = processor->getLCMSDataset();
    if ( dset ) {
        adcontrols::Chromatogram c;
        if ( dset->getTIC( 0, c ) ) {
            c.addDescription( adcontrols::Description( L"filename", file.filename() ) );
            pImpl_->setData( c, filename );
        }
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


///////////////////////////

void
ChromatogramWndImpl::setData( const adcontrols::Chromatogram& c, const QString& )
{
    chroWidget_->setData( c );
}
