/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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
#include <adcontrols/waveform.hpp>
#include <adportable/debug.hpp>
#include <adutils/processeddata.hpp>
#include <adwplot/picker.hpp>
#include <portfolio/folium.hpp>
#include <portfolio/folder.hpp>
#include <adwplot/chromatogramwidget.hpp>
#include <adwplot/spectrumwidget.hpp>
#include <coreplugin/minisplitter.h>
#include <qwt_scale_widget.h>
#include <QBoxLayout>
#include <QMenu>
#include <boost/variant.hpp>
#include "selchanged.hpp"

using namespace dataproc;

namespace dataproc {
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
        portfolio::Folder& folder_;
        selProcessed( Wnd& wnd, portfolio::Folder& folder ) : wnd_( wnd )
                                                            , folder_( folder ) {
        }
        template<typename T> void operator ()( T& ) const {
            adportable::debug(__FILE__, __LINE__) << "Unhandled data: " << typeid(T).name() << " received.";
        }
        
        void operator () ( adutils::MassSpectrumPtr& ptr ) const {   
            wnd_.idSpectrumFolium( folder_.id() );
            wnd_.draw2( ptr );
        }
        
        void operator () ( adutils::ChromatogramPtr& ptr ) const {
            wnd_.idChromatogramFolium( folder_.id() );
            wnd_.draw( ptr );
        }
        
        void operator () ( adutils::PeakResultPtr& ptr ) const {
            wnd_.draw( ptr );
        }
        
    };

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
			connect( pImpl_->profileSpectrum_, SIGNAL( onSelected( const QRectF& ) ), this, SLOT( selectedOnProfile( const QRectF& ) ) );
        }

        if ( ( pImpl_->processedSpectrum_ = new adwplot::SpectrumWidget(this) ) ) {
            pImpl_->processedSpectrum_->setMinimumHeight( 80 );
			connect( pImpl_->processedSpectrum_, SIGNAL( onSelected( const QPointF& ) ), this, SLOT( selectedOnProcessed( const QPointF& ) ) );
			connect( pImpl_->processedSpectrum_, SIGNAL( onSelected( const QRectF& ) ), this, SLOT( selectedOnProcessed( const QRectF& ) ) );
        }
        splitter->addWidget( pImpl_->ticPlot_ );
        splitter->addWidget( pImpl_->profileSpectrum_ );
        splitter->addWidget( pImpl_->processedSpectrum_ );
        splitter->setOrientation( Qt::Vertical );

        //static_cast<QwtPlot *>( pImpl_->profileSpectrum_ )->axisWidget( QwtPlot::yLeft )->setMargin( 80 );
        //static_cast<QwtPlot *>( pImpl_->processedSpectrum_ )->axisWidget( QwtPlot::yLeft )->setMargin( 80 );
        //static_cast<QwtPlot *>( pImpl_->profileSpectrum_ )->axisWidget( QwtPlot::yLeft )->scaleDraw()->setLength( 300 );
        //static_cast<QwtPlot *>( pImpl_->processedSpectrum_ )->axisWidget( QwtPlot::yLeft )->scaleDraw()->setLength( 300 );

        pImpl_->profileSpectrum_->link( pImpl_->processedSpectrum_ );
        pImpl_->processedSpectrum_->link( pImpl_->profileSpectrum_ );

        pImpl_->processedSpectrum_->setContextMenuPolicy( Qt::CustomContextMenu );
		connect( pImpl_->processedSpectrum_, SIGNAL( customContextMenuRequested( QPoint ) )
                 , this, SLOT( handleCustomMenuOnProcessedSpectrum( const QPoint& ) ) );
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
#if 1 
    //---> for debug
    adcontrols::MassSpectrum ms2( ms );
    adcontrols::waveform::fft::lowpass_filter( ms2, 100.0e6 );  // 100MHz low pass filter
    pImpl_->profileSpectrum_->setData( ms2, drawIdx1_++ );
    // <--
#endif
}

void
MSProcessingWnd::draw2( adutils::MassSpectrumPtr& ptr )
{
    pProcessedSpectrum_ = ptr;
    pImpl_->processedSpectrum_->setData( *pProcessedSpectrum_, drawIdx2_++ );
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
MSProcessingWnd::idSpectrumFolium( const std::wstring& id )
{
    idSpectrumFolium_ = id;
}

void
MSProcessingWnd::idChromatogramFolium( const std::wstring& id )
{
    idChromatogramFolium_ = id;
}

void
MSProcessingWnd::handleSessionAdded( Dataprocessor * processor )
{
    const adcontrols::LCMSDataset * dset = processor->getLCMSDataset();
    if ( dset ) {
        adcontrols::Chromatogram c;
        int fcn = 0;
        while ( dset->getTIC( fcn, c ) ) {
            if ( c.isConstantSampledData() )
                c.getTimeArray();
            c.addDescription( adcontrols::Description( L"filename", processor->file().filename() ) );
            pImpl_->ticPlot_->setData( c, fcn );
            ++fcn;
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

        auto data = adutils::ProcessedData::toVariant( static_cast<boost::any&>( folium ) );
        
        if ( boost::apply_visitor( selChanged<MSProcessingWnd>(*this), data ) ) {

            idActiveFolium_ = folium.id();
            
            portfolio::Folio attachments = folium.attachments();
            for ( portfolio::Folio::iterator it = attachments.begin(); it != attachments.end(); ++it ) {
                auto contents = adutils::ProcessedData::toVariant( static_cast<boost::any&>( *it ) );
                boost::apply_visitor( selProcessed<MSProcessingWnd>( *this, folder ), contents );
            }
        }
    }
}

void
MSProcessingWnd::handleApplyMethod( const adcontrols::ProcessMethod& )
{
}

void
MSProcessingWnd::handleCustomMenuOnProcessedSpectrum( const QPoint& pos )
{
	// This is conflicting with picker's action, so it has moved to range selection slots
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
    adportable::debug(__FILE__, __LINE__) << "MSProcessingWnd::selectedOnProfile: " << pos.x() << ", " << pos.y();
}

void
MSProcessingWnd::selectedOnProfile( const QRectF& rect )
{
}


void
MSProcessingWnd::selectedOnProcessed( const QPointF& pos )
{
    adportable::debug(__FILE__, __LINE__) << "MSProcessingWnd::selectedOnProcessed: " << pos.x() << ", " << pos.y();
}

void
MSProcessingWnd::selectedOnProcessed( const QRectF& rect )
{
	QMenu menu;

	std::vector< QAction * > actions;
	actions.push_back( menu.addAction( "Create chromatograms" ) );

	QAction * selectedItem = menu.exec( QCursor::pos() );
	auto it = std::find_if( actions.begin(), actions.end(), [selectedItem]( const QAction *item ){
            return item == selectedItem; }
        );
	if ( it != actions.end() ) {
		QRectF rc = pImpl_->processedSpectrum_->zoomRect();
        if ( *it == actions[ 0 ] )
			DataprocPlugin::instance()->handleCreateChromatograms( *pProcessedSpectrum_, rc.x(), rc.x() + rc.width() );
	}
}
