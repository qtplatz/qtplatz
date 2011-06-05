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

#include "msprocessingwnd.hpp"
#include "dataprocessor.hpp"
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/datafile.hpp>
#include <adportable/debug.hpp>
#include <adutils/processeddata.hpp>
#include <portfolio/folium.hpp>
#include <portfolio/folder.hpp>
#include <adwplot/chromatogramwidget.hpp>
#include <adwplot/spectrumwidget.hpp>
#include <coreplugin/minisplitter.h>
#include <QBoxLayout>
#include <boost/variant.hpp>
#include "selchanged.hpp"

using namespace dataproc;
using namespace dataproc::internal;

namespace dataproc {
    namespace internal {
        class MSProcessingWndImpl {
        public:
            ~MSProcessingWndImpl() {}
            MSProcessingWndImpl() : ticPlot_(0)
                                  , profileSpectrum_(0)
                                  , processedSpectrum_(0) {
            }

            adwplot::ChromatogramWidget * ticPlot_;
            adwplot::SpectrumWidget * profileSpectrum_;
            adwplot::SpectrumWidget * processedSpectrum_;

        };

        //---------------------------------------------------------
        template<class Wnd> struct selProcessed : public boost::static_visitor<void> {
            Wnd& wnd_;
            selProcessed( Wnd& wnd ) : wnd_(wnd) {}

            template<typename T> void operator ()( T& ) const {
            }

            void operator () ( adutils::MassSpectrumPtr& ptr ) const {   
                wnd_.draw2( ptr );
            }

            void operator () ( adutils::ChromatogramPtr& ptr ) const {
                wnd_.draw( ptr );
            }
        };
        //-----
    }
}

MSProcessingWnd::MSProcessingWnd(QWidget *parent) :
    QWidget(parent)
{
    init();
}

void
MSProcessingWnd::init()
{
    pImpl_.reset( new MSProcessingWndImpl );
    Core::MiniSplitter * splitter = new Core::MiniSplitter;
    if ( splitter ) {
        if ( ( pImpl_->ticPlot_ = new adwplot::ChromatogramWidget(this) ) ) {
            pImpl_->ticPlot_->setMinimumHeight( 80 );
        }
	
        if ( ( pImpl_->profileSpectrum_ = new adwplot::SpectrumWidget(this) ) ) {
            pImpl_->profileSpectrum_->setMinimumHeight( 80 );
        }

        if ( ( pImpl_->processedSpectrum_ = new adwplot::SpectrumWidget(this) ) ) {
            pImpl_->processedSpectrum_->setMinimumHeight( 80 );
        }
        splitter->addWidget( pImpl_->ticPlot_ );
        splitter->addWidget( pImpl_->profileSpectrum_ );
        splitter->addWidget( pImpl_->processedSpectrum_ );
        splitter->setOrientation( Qt::Vertical );

        pImpl_->profileSpectrum_->link( pImpl_->processedSpectrum_ );
        pImpl_->processedSpectrum_->link( pImpl_->profileSpectrum_ );

        pImpl_->processedSpectrum_->setContextMenuPolicy( Qt::CustomContextMenu );
        connect( pImpl_->processedSpectrum_, SIGNAL( customContextMenuRequested( QPoint ) ), this, SLOT( ctxMenu1( QPoint ) ) );
    }

    QBoxLayout * toolBarAddingLayout = new QVBoxLayout( this );
    toolBarAddingLayout->setMargin(0);
    toolBarAddingLayout->setSpacing(0);
    //toolBarAddingLayout->addWidget( toolBar );
    toolBarAddingLayout->addWidget( splitter );
    //toolBarAddingLayout->addWidget( toolBar2 );

}

void
MSProcessingWnd::draw1( adutils::MassSpectrumPtr& ptr )
{
    adcontrols::MassSpectrum& ms = *ptr;
    pImpl_->profileSpectrum_->setData( ms, drawIdx1_++ );
}

void
MSProcessingWnd::draw2( adutils::MassSpectrumPtr& ptr )
{
    adcontrols::MassSpectrum& ms = *ptr;
    pImpl_->processedSpectrum_->setData( ms, drawIdx2_++ );
}

void
MSProcessingWnd::draw( adutils::ChromatogramPtr& ptr )
{
    adcontrols::Chromatogram& c = *ptr;
    pImpl_->ticPlot_->setData( c );
}

void
MSProcessingWnd::handleSessionAdded( Dataprocessor * processor )
{
    const adcontrols::LCMSDataset * dset = processor->getLCMSDataset();
    if ( dset ) {
        adcontrols::Chromatogram c;
        if ( dset->getTIC( 0, c ) ) {
            if ( c.isConstantSampledData() )
                c.getTimeArray();
            c.addDescription( adcontrols::Description( L"filename", processor->file().filename() ) );
            pImpl_->ticPlot_->setData( c );
        }
    }
}

void
MSProcessingWnd::handleSelectionChanged( Dataprocessor* /* processor */, portfolio::Folium& folium )
{
    drawIdx1_ = 0;
    drawIdx2_ = 0;

#if defined DEBUG
    std::string typname = static_cast<boost::any&>( folium ).type().name();
    adportable::debug(__FILE__, __LINE__) << "handleSelectionChanged: " << typname << " id=" << folium.id();
#endif

    portfolio::Folder folder = folium.getParentFolder();
    if ( folder && ( folder.name() == L"Spectra" || folder.name() == L"Chromatograms" ) ) {

        adutils::ProcessedData::value_type data = adutils::ProcessedData::toVariant( static_cast<boost::any&>( folium ) );

        int which = data.which();

        if ( boost::apply_visitor( selChanged<MSProcessingWnd>(*this), data ) ) {
            idActiveFolium_ = folium.id();

            portfolio::Folio attachments = folium.attachments();
            for ( portfolio::Folio::iterator it = attachments.begin(); it != attachments.end(); ++it ) {
                adutils::ProcessedData::value_type contents = adutils::ProcessedData::toVariant( static_cast<boost::any&>( *it ) );
                boost::apply_visitor( selProcessed<MSProcessingWnd>( *this ), contents );
            }
        }
    }
}

void
MSProcessingWnd::ctxMenu1( const QPoint& )
{
}
