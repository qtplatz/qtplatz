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
#include "dataprocplugin.hpp"
#include "dataprocessor.hpp"
#include "sessionmanager.hpp"
#include "datafileobserver_i.hpp"
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
#include <QMenu>
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
#if defined _DEBUG && defined DEBUG
				adportable::debug() << typeid(T).name();
#endif
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
			connect( pImpl_->ticPlot_, SIGNAL( onSelected( const QPointF& ) ), this, SLOT( selectedOnChromatogram( const QPointF& ) ) );
			connect( pImpl_->ticPlot_, SIGNAL( onSelected( const QRectF& ) ), this, SLOT( selectedOnChromatogram( const QRectF& ) ) );
        }
	
        if ( ( pImpl_->profileSpectrum_ = new adwplot::SpectrumWidget(this) ) ) {
            pImpl_->profileSpectrum_->setMinimumHeight( 80 );
			connect( pImpl_->profileSpectrum_, SIGNAL( onSelected( const QPointF& ) ), this, SLOT( selectedOnProfile( const QPointF& ) ) );
        }

        if ( ( pImpl_->processedSpectrum_ = new adwplot::SpectrumWidget(this) ) ) {
            pImpl_->processedSpectrum_->setMinimumHeight( 80 );
			connect( pImpl_->processedSpectrum_, SIGNAL( onSelected( const QPointF& ) ), this, SLOT( selectedOnProcessed( const QPointF& ) ) );
        }
        splitter->addWidget( pImpl_->ticPlot_ );
        splitter->addWidget( pImpl_->profileSpectrum_ );
        splitter->addWidget( pImpl_->processedSpectrum_ );
        splitter->setOrientation( Qt::Vertical );

        pImpl_->profileSpectrum_->link( pImpl_->processedSpectrum_ );
        pImpl_->processedSpectrum_->link( pImpl_->profileSpectrum_ );

        pImpl_->processedSpectrum_->setContextMenuPolicy( Qt::CustomContextMenu );
		connect( pImpl_->processedSpectrum_, SIGNAL( customContextMenuRequested( QPoint ) ), this, SLOT( onCustomMenuOnProcessedSpectrum( const QPoint& ) ) );
    }

    QBoxLayout * toolBarAddingLayout = new QVBoxLayout( this );
    toolBarAddingLayout->setMargin(0);
    toolBarAddingLayout->setSpacing(0);
    toolBarAddingLayout->addWidget( splitter );
}

void
MSProcessingWnd::draw1( adutils::MassSpectrumPtr& ptr )
{
    adcontrols::MassSpectrum& ms = *ptr;
    pImpl_->profileSpectrum_->setData( ms, drawIdx1_++ );
    pImpl_->processedSpectrum_->clear();
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
MSProcessingWnd::draw( adutils::PeakResultPtr& ptr )
{
	pImpl_->ticPlot_->setData( *ptr );
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

    portfolio::Folder folder = folium.getParentFolder();
    if ( folder && ( folder.name() == L"Spectra" || folder.name() == L"Chromatograms" ) ) {

#if defined DEBUG || defined _DEBUG
        boost::any& any = static_cast<boost::any&>( folium );
        std::string type = any.type().name();
        adportable::debug(__FILE__, __LINE__) << "handleSelectionChanged got data type: " << type;
#endif

        adutils::ProcessedData::value_type data = adutils::ProcessedData::toVariant( static_cast<boost::any&>( folium ) );

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
MSProcessingWnd::onCustomMenuOnProcessedSpectrum( const QPoint& pos )
{
	QPoint globalPos = pImpl_->processedSpectrum_->mapToGlobal(pos);
    // for QAbstractScrollArea and derived classes you would use:
    // QPoint globalPos = myWidget->viewport()->mapToGlobal(pos); 
/*
    QMenu myMenu;
    myMenu.addAction("Menu Item 1");
    myMenu.addAction("Menu Item 2");

	// QAction* selectedItem = myMenu.exec( globalPos );
    if (selectedItem)    {
        // something was chosen, do stuff
    }
*/
}

void
MSProcessingWnd::selectedOnChromatogram( const QPointF& pos )
{
	DataprocPlugin::instance()->onSelectTimeRangeOnChromatogram( pos.x(), pos.x() ); 
}

void
MSProcessingWnd::selectedOnChromatogram( const QRectF& rect )
{
	DataprocPlugin::instance()->onSelectTimeRangeOnChromatogram( rect.x(), rect.x() + rect.width() ); 
}

void
MSProcessingWnd::selectedOnProfile( const QPointF& pos )
{
	std::cout << "MSProcessingWnd::selectedOnProfile: " << pos.x() << ", " << pos.y() << std::endl;
}

void
MSProcessingWnd::selectedOnProcessed( const QPointF& pos )
{
	std::cout << "MSProcessingWnd::selectedOnProcessed: " << pos.x() << ", " << pos.y() << std::endl;
}
