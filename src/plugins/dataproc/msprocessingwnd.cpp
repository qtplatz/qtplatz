/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC
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
#include "document.hpp"
#include "dataprocplugin.hpp"
#include "dataprocessor.hpp"
#include "dataprocessworker.hpp"
#include "sessionmanager.hpp"
#include "mainwindow.hpp"
#include <adcontrols/annotation.hpp>
#include <adcontrols/annotations.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/constants.hpp>
#include <adcontrols/datafile.hpp>
#include <adcontrols/datareader.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/descriptions.hpp>
#include <adcontrols/histogram.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/lockmass.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/massspectrometerbroker.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/mspeakinfoitem.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/peakresult.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/samplinginfo.hpp>
#include <adcontrols/targeting.hpp>
#include <adcontrols/waveform_filter.hpp>
#include <adcontrols/scanlaw.hpp>
#include <adlog/logger.hpp>
#include <adfs/sqlite.hpp>
#include <adplot/picker.hpp>
#include <adplot/peakmarker.hpp>
#include <adplot/chromatogramwidget.hpp>
#include <adplot/spectrumwidget.hpp>
#include <adplot/tracewidget.hpp>
#include <adplot/zoomer.hpp>
#include <adportable/scoped_debug.hpp>
#include <adportable/spectrum_processor.hpp>
#include <adportable/array_wrapper.hpp>
#include <adportable/xml_serializer.hpp>
#include <adportable/fft.hpp>
#include <adportable/timesquaredscanlaw.hpp>
#include <adportable/float.hpp>
#include <adpublisher/printer.hpp>
#include <adutils/processeddata.hpp>
#include <adwidgets/filedialog.hpp>
#include <adwidgets/scanlawdialog.hpp>
#include <adwidgets/scanlawdialog2.hpp>
#include <adportfolio/portfolio.hpp>
#include <adportfolio/folium.hpp>
#include <adportfolio/folder.hpp>
#include <qtwrapper/xmlformatter.hpp>
#include <qtwrapper/font.hpp>
#include <qtwrapper/waitcursor.hpp>
#include <xmlparser/pugixml.hpp>
#include <coreplugin/minisplitter.h>
#include <qwt_scale_widget.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_renderer.h>
#include <qwt_plot_marker.h>
#include <qwt_scale_engine.h>
#include <qwt_symbol.h>
#include <QApplication>
#include <QBoxLayout>
#include <QCheckBox>
#include <QClipboard>
#include <QMenu>
#include <QPrinter>
#include <QSettings>
#include <QSlider>
#include <QSvgGenerator>
#include <QTextDocument>
#include <boost/exception/all.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/variant.hpp>
#include "selchanged.hpp"
#include <algorithm>
#include <array>
#include <complex>
#include <functional>
#include <numeric>
#include <sstream>

using namespace dataproc;

namespace dataproc {

    class MSProcessingWndImpl {
    public:
        MSProcessingWndImpl() : ticPlot_(0)
                              , profileSpectrum_(0)
                              , processedSpectrum_(0)
                              , pwplot_( new adplot::TraceWidget )
                              , is_time_axis_( false )
                              , hasHistogram_( false ) {
        }

        void currentChanged( const adcontrols::MSPeakInfoItem& pk ) {
            if ( profile_marker_ ) {
                profile_marker_->setPeak( pk, is_time_axis_, adcontrols::metric::micro );
                profileSpectrum_->replot();
            }
            if ( processed_marker_ ) {
                processed_marker_->setPeak( pk, is_time_axis_, adcontrols::metric::micro );
                processedSpectrum_->replot();
            }
        }

        void currentChanged( const adcontrols::MassSpectrum& ms, int idx ) {
            if ( profile_marker_ ) {
                profile_marker_->setPeak( ms, idx, is_time_axis_, adcontrols::metric::micro );
                profileSpectrum_->replot();
            }
            if ( processed_marker_ ) {
                processed_marker_->setPeak( ms, idx, is_time_axis_, adcontrols::metric::micro );
                processedSpectrum_->replot();
            }
        }

        void set_time_axis( bool isTime ) { 
            is_time_axis_ = isTime;
        }

        void focusedFcn( int fcn ) {
            profileSpectrum_->setFocusedFcn( fcn );
            processedSpectrum_->setFocusedFcn( fcn );
        }

        bool ticFinder3( const adcontrols::LCMSDataset * rawfile, double x, size_t& pos, int& index, int& rep, int& fcn, double& minutes ) {
            size_t idx = 0;
            double error = 999.0;
            double seconds = adcontrols::Chromatogram::toSeconds( x );
            while ( auto reader = rawfile->dataReader( idx++ ) ) {
                if ( reader->fcnCount() ) {
                    if ( auto it = reader->findPos( seconds ) ) {
                        if ( error > std::abs( it->time_since_inject() - seconds ) ) {
                            error = std::abs( it->time_since_inject() - seconds );
                            pos = it->pos();
                            minutes = it->time_since_inject();
                            rep = 0;
                            fcn = it->fcn();
                        }
                    }
                }
            }
            return true;
        }

        bool ticFinder2( const adcontrols::LCMSDataset * rawfile, double x, size_t& pos, int& index, int& rep, int& fcn, double& seconds ) {
            index = 0;
            fcn = (-1);
            seconds = 0;
            
            pos = rawfile->posFromTime( adcontrols::Chromatogram::toSeconds( x ) );
            if ( rawfile->index( pos, index, fcn, rep ) ) {
                seconds = rawfile->timeFromPos( pos ); // adcontrols::Chromatogram::toMinutes( rawfile->timeFromPos( pos ) );
                return true;
            }

            int traceId = -1;
            index = -1;

            double s = adcontrols::Chromatogram::toSeconds( x );
            double t = std::numeric_limits<double>::max();
            for ( auto& chro: checkedChromatograms_ ) {
                if ( auto pchr = chro.second.lock() ) {
                    if ( const double * times = pchr->getTimeArray() ) {
                        int idx = int( std::distance( times, std::lower_bound( times, times + pchr->size(), s ) ) );
                        if ( std::abs( t - s ) > std::abs( times[ idx ] - s ) ) {
                            traceId = chro.first;
                            index = idx;
                            fcn = pchr->fcn();
                            t = times[ idx ];
                        }
                    }
                }
            }
            seconds = t; // adcontrols::Chromatogram::toMinutes( t );
            return ( traceId >= 0 ) && ( index >= 0 );
        }

        bool ticTracker( const QPointF& pos, QwtText& text ) {
            if ( auto dp = SessionManager::instance()->getActiveDataprocessor() ) {
                if ( auto rawfile = dp->rawdata() ) {
                    
                    size_t npos(0);
                    int fcn(0), rep(0), index(0);
                    double minutes(0);
                    
                    if ( rawfile->dataformat_version() >= 3 ) {
                        return true;
                    } else {
                        if ( ticFinder2( rawfile, pos.x(), npos, index, rep, fcn, minutes ) ) {
                            QString ammend = QString::fromStdString(
                                ( boost::format( "[data#%d idx:%d [rep:%d] fcn:%d]" ) % npos % index % rep % fcn ).str() );
                            text.setText( QString( "%1 %2" ).arg( text.text(), ammend ), QwtText::RichText );
                            return true;
                        }
                    }
                }
            }
            return false;
        }

        void clearChromatograms() {
            checkedChromatograms_.clear();
        }

        void setCheckedChromatogram( std::shared_ptr< adcontrols::Chromatogram >& ptr, int idx ) {
            checkedChromatograms_[ idx ] = ptr;
        }
        
        adplot::ChromatogramWidget * ticPlot_;
        adplot::SpectrumWidget * profileSpectrum_;
        adplot::SpectrumWidget * processedSpectrum_;
        adplot::TraceWidget * pwplot_;
        std::shared_ptr< adplot::PeakMarker > profile_marker_;
        std::shared_ptr< adplot::PeakMarker > processed_marker_;
        std::map< int, std::weak_ptr< adcontrols::Chromatogram > > checkedChromatograms_;
        bool is_time_axis_;
        bool hasHistogram_;
    };

}

MSProcessingWnd::MSProcessingWnd(QWidget *parent) : QWidget(parent)
                                                  , drawIdx1_( 0 )
                                                  , drawIdx2_( 0 )
                                                  , axis_(adcontrols::hor_axis_mass)
{
    init();
}

void
MSProcessingWnd::init()
{
    pImpl_ = std::make_shared< MSProcessingWndImpl >();

    Core::MiniSplitter * splitter = new Core::MiniSplitter;
    if ( splitter ) {

        if ( ( pImpl_->ticPlot_ = new adplot::ChromatogramWidget(this) ) ) {
            pImpl_->ticPlot_->setMinimumHeight( 80 );
			connect( pImpl_->ticPlot_, SIGNAL( onSelected( const QRectF& ) ), this, SLOT( selectedOnChromatogram( const QRectF& ) ) );
            pImpl_->ticPlot_->register_tracker( [=]( const QPointF& pos, QwtText& text ){ return pImpl_->ticTracker( pos, text ); } );
        }
	
        if ( ( pImpl_->profileSpectrum_ = new adplot::SpectrumWidget(this) ) ) {
            pImpl_->profileSpectrum_->setMinimumHeight( 80 );
            using adplot::SpectrumWidget;
            connect( pImpl_->profileSpectrum_, &SpectrumWidget::onSelected, this, &MSProcessingWnd::selectedOnProfile );
            pImpl_->profile_marker_ = std::make_shared< adplot::PeakMarker >();
            pImpl_->profile_marker_->attach( pImpl_->profileSpectrum_ );
            pImpl_->profile_marker_->visible( true );
            pImpl_->profile_marker_->setYAxis( QwtPlot::yLeft );
        }

        if ( ( pImpl_->processedSpectrum_ = new adplot::SpectrumWidget(this) ) ) {
            pImpl_->processedSpectrum_->setMinimumHeight( 80 );
            using adplot::SpectrumWidget;
            connect( pImpl_->processedSpectrum_, static_cast< void(SpectrumWidget::*)(const QRectF&) >(&SpectrumWidget::onSelected)
                     , this, &MSProcessingWnd::selectedOnProcessed );
            adplot::Zoomer * zoomer = pImpl_->processedSpectrum_->zoomer();
            connect( zoomer, &adplot::Zoomer::zoomed, this, &MSProcessingWnd::handleZoomedOnSpectrum );

            pImpl_->processed_marker_ = std::make_shared< adplot::PeakMarker >();
            pImpl_->processed_marker_->attach( pImpl_->processedSpectrum_ );
            pImpl_->processed_marker_->visible( true );
            pImpl_->processed_marker_->setYAxis( QwtPlot::yLeft );
        }
		pImpl_->ticPlot_->axisWidget( QwtPlot::yLeft )->scaleDraw()->setMinimumExtent( 80 );
		pImpl_->profileSpectrum_->axisWidget( QwtPlot::yLeft )->scaleDraw()->setMinimumExtent( 80 );
		pImpl_->processedSpectrum_->axisWidget( QwtPlot::yLeft )->scaleDraw()->setMinimumExtent( 80 );

        pImpl_->profileSpectrum_->axisWidget( QwtPlot::yRight )->scaleDraw()->setMinimumExtent( 60 );
		pImpl_->processedSpectrum_->axisWidget( QwtPlot::yRight )->scaleDraw()->setMinimumExtent( 60 );

        pImpl_->pwplot_->setMinimumHeight( 80 );
        pImpl_->pwplot_->axisWidget( QwtPlot::yLeft )->scaleDraw()->setMinimumExtent( 80 );
        pImpl_->pwplot_->axisWidget( QwtPlot::yLeft )->scaleDraw()->setMinimumExtent( 60 );
        pImpl_->pwplot_->xBottomTitle( "Frequency (MHz)" );
        pImpl_->pwplot_->yLeftTitle( "Power" );
        connect( pImpl_->pwplot_, SIGNAL( onSelected( const QRectF& ) ), this, SLOT( selectedOnPowerPlot( const QRectF& ) ) );

		splitter->addWidget( pImpl_->ticPlot_ );
        splitter->addWidget( pImpl_->profileSpectrum_ );
        splitter->addWidget( pImpl_->processedSpectrum_ );
        splitter->addWidget( pImpl_->pwplot_ );
        splitter->setOrientation( Qt::Vertical );

        splitter->setChildrenCollapsible( true );
        pImpl_->pwplot_->hide();

        pImpl_->profileSpectrum_->link( pImpl_->processedSpectrum_ );
        //pImpl_->processedSpectrum_->link( pImpl_->profileSpectrum_ );

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
MSProcessingWnd::draw_histogram( portfolio::Folium& folium, adutils::MassSpectrumPtr& hist )
{
    if ( ! hist->isCentroid() ) {
        draw_profile( folium.id(), hist );
        return;
    }

    pImpl_->hasHistogram_ = true;

    std::shared_ptr< adcontrols::MassSpectrum > profile;
    
    if ( Dataprocessor * dp = SessionManager::instance()->getActiveDataprocessor() ) {

        pProfileSpectrum_ = std::make_pair( folium.id(), hist ); // sticked
        
        auto att = dp->findProfiledHistogram( folium );
        if ( !att )
            att = dp->addProfiledHistogram( folium );
        if ( att ) {
            profile = portfolio::get< adcontrols::MassSpectrumPtr >( att );
            pProfileHistogram_ = std::make_pair( att.id(), profile ); // profiled
        } else {
            pProfileHistogram_.second.reset();
        }
    }

    if ( axis_ == adcontrols::hor_axis_mass ) {
        if ( hist->size() > 0
             && adportable::compare<double>::approximatelyEqual( hist->getMass( hist->size() - 1 ), hist->getMass( 0 ) ) ) {
            // Spectrum has no mass assigned
            MainWindow::instance()->setSpectrumAxisChoice( adcontrols::hor_axis_time );
        }
    }

    pImpl_->profileSpectrum_->setData( profile, static_cast<int>(drawIdx1_++) );
    pImpl_->profileSpectrum_->setAlpha( drawIdx1_ - 1, 0x60 );

    pImpl_->profileSpectrum_->setData( hist, static_cast<int>(drawIdx1_++) );
    
    QString title = QString("[%1]").arg( MainWindow::makeDisplayName( idSpectrumFolium_ ) );
	for ( auto text: hist->getDescriptions() )
		title += QString::fromStdWString( std::wstring( text.text() ) + L", " );

	pImpl_->profileSpectrum_->setTitle( title );
    pImpl_->processedSpectrum_->clear();

	drawIdx2_ = 0;
}

void
MSProcessingWnd::draw_profile( const std::wstring& guid, adutils::MassSpectrumPtr& ptr )
{
    pProfileSpectrum_ = std::make_pair( guid, ptr );
    pProfileHistogram_.second.reset();

    if ( pImpl_->hasHistogram_ ) {
        pImpl_->profileSpectrum_->clear();
        pImpl_->hasHistogram_ = false;
    }

    if ( axis_ == adcontrols::hor_axis_mass ) {
        if ( ptr->size() > 0
             && adportable::compare<double>::approximatelyEqual( ptr->getMass( ptr->size() - 1 ), ptr->getMass( 0 ) ) ) {
                // Spectrum has no mass assigned
                MainWindow::instance()->setSpectrumAxisChoice( adcontrols::hor_axis_time );
        }
    }

    pImpl_->profileSpectrum_->setData( ptr, static_cast<int>(drawIdx1_++) );
    QString title = QString("[%1]").arg( MainWindow::makeDisplayName( idSpectrumFolium_ ) );
	for ( auto text: ptr->getDescriptions() )
		title += QString::fromStdWString( std::wstring( text.text() ) + L", " );
	pImpl_->profileSpectrum_->setTitle( title );
    pImpl_->processedSpectrum_->clear();
	drawIdx2_ = 0;
}

void
MSProcessingWnd::draw1()
{
    if ( auto ptr = pProfileSpectrum_.second.lock() ) {
        if ( drawIdx1_ )
            --drawIdx1_;
        pImpl_->profileSpectrum_->setData( ptr, static_cast<int>(drawIdx1_++) );

        QString title = QString("[%1]&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;").arg( MainWindow::makeDisplayName( idSpectrumFolium_ ) );
        for ( auto text: ptr->getDescriptions() )
            title += QString::fromStdWString( std::wstring( text.text() ) + L", " );

        pImpl_->profileSpectrum_->setTitle( title );
        pImpl_->processedSpectrum_->clear();
    }
}

void
MSProcessingWnd::draw2( adutils::MassSpectrumPtr& ptr )
{
    int idx = int( drawIdx2_++ );
    if ( ptr->isCentroid() )
        pImpl_->processedSpectrum_->setData( ptr, idx, false );
    else {
        pImpl_->processedSpectrum_->setData( ptr, idx, true );
        pImpl_->processedSpectrum_->setAlpha( idx, 0x20 );

        pImpl_->processedSpectrum_->enableAxis( QwtPlot::yRight, true );
        pImpl_->profileSpectrum_->enableAxis( QwtPlot::yRight, true );
    }
}

void
MSProcessingWnd::draw( adutils::ChromatogramPtr& ptr, int idx )
{
    pImpl_->ticPlot_->setData( ptr, idx );
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
    portfolio::Portfolio portfolio = processor->getPortfolio();

    if ( const adcontrols::LCMSDataset * dset = processor->rawdata() ) {

        portfolio::Folder folder = portfolio.findFolder( L"Chromatograms" );
        if ( folder.nil() )
            folder = processor->getPortfolio().addFolder( L"Chromatograms" );

        if ( dset->dataformat_version() >= 3 ) {

            adcontrols::ProcessMethod m;
            MainWindow::instance()->getProcessMethod( m );

            auto vec = dset->dataReaders();
            for ( auto& reader : vec ) {
                for ( int fcn = 0; fcn < reader->fcnCount(); ++fcn ) {
                    if ( auto tic = reader->TIC( fcn ) ) {
                        auto folium = folder.findFoliumByName( ( boost::wformat( L"%1%/%2%.%3%" )
                                                                 % adcontrols::Chromatogram::make_folder_name( tic->getDescriptions() )
                                                                 % L"TIC" % ( fcn + 1 ) ).str() );
                        if ( folium.nil() ) {
                            adcontrols::Chromatogram c = *tic;
                            c.addDescription( adcontrols::description( L"acquire.title", ( boost::wformat( L"TIC.%1%" ) % ( fcn + 1 ) ).str() ) );
                            portfolio::Folium folium = processor->addChromatogram( c, m, true );
                        }
                        processor->setCurrentSelection( folium );
                    }
                }
            }
        } else {
            size_t nfcn = dset->getFunctionCount();
            for ( size_t fcn = 0; fcn < nfcn; ++fcn ) {
                std::wstring title = ( boost::wformat( L"TIC.%1%" ) % ( fcn + 1 ) ).str();

                portfolio::Folium folium = folder.findFoliumByName( std::wstring( L"TIC/" ) + title );
                if ( folium.nil() ) {   // add TIC if not yet added
                    adcontrols::Chromatogram c;
                    if ( dset->getTIC( static_cast<int>( fcn ), c ) ) {
                        if ( c.isConstantSampledData() )
                            c.getTimeArray();
                        c.addDescription( adcontrols::description( L"acquire.title", title ) );
                        adcontrols::ProcessMethod m;
                        MainWindow::instance()->getProcessMethod( m );
                        folium = processor->addChromatogram( c, m, true );  // force checked
                    }
                }
                if ( folium.attribute( L"protoId" ).empty() )
                    folium.setAttribute( L"protoId", ( boost::wformat( L"%d" ) % fcn ).str() );
            }
            if ( portfolio::Folium folium = folder.findFoliumByName( L"TIC/TIC.1" ) ) {
                qtwrapper::waitCursor wait;
                if ( folium.empty() )
                    processor->fetch( folium );
                processor->setCurrentSelection( folium );
            }
        }
    }

    // show first spectrum on tree by default
    portfolio::Folder spectra = portfolio.findFolder( L"Spectra" );
    if ( !spectra.nil() ) {
        portfolio::Folio folio = spectra.folio();
        if ( !folio.empty() ) {
            processor->fetch( folio [ 0 ] );
            processor->setCurrentSelection( folio [ 0 ] );
        }
    }
}

void
MSProcessingWnd::handleZoomedOnSpectrum( const QRectF& rc )
{
    MainWindow::instance()->zoomedOnSpectrum( rc );
}

void
MSProcessingWnd::handleProcessed( Dataprocessor* processor, portfolio::Folium& folium )
{
    handleSelectionChanged( processor, folium );
}

void
MSProcessingWnd::handleSelectionChanged( Dataprocessor* processor, portfolio::Folium& folium )
{
    drawIdx1_ = 0;
    drawIdx2_ = 0;

    if ( portfolio::Folder folder = folium.parentFolder() ) {
        
        if ( folder.name() == L"Spectra" ) { //|| folder.name() == L"Chromatograms" ) {

            if ( portfolio::is_type< adcontrols::MassSpectrumPtr >( folium ) ) {

                pProcessedSpectrum_ = std::make_pair( std::wstring(), std::shared_ptr< adcontrols::MassSpectrum >( 0 ) );
                pProfileSpectrum_ = std::make_pair( std::wstring(), std::shared_ptr< adcontrols::MassSpectrum >( 0 ) );
                pkinfo_ = std::make_pair( std::wstring(), std::shared_ptr< adcontrols::MSPeakInfo >( 0 ) );
                targeting_ = std::make_pair( std::wstring(), std::shared_ptr< adcontrols::Targeting >( 0 ) );

                if ( auto ptr = portfolio::get< adcontrols::MassSpectrumPtr >( folium ) ) {

                    idActiveFolium_ = folium.id();
                    idSpectrumFolium_ = folium.id();

                    if ( ptr->isCentroid() )
                        draw_histogram( folium, ptr );
                    else
                        draw_profile( folium.id(), ptr );

                    if ( auto fcentroid = portfolio::find_first_of( folium.attachments(), []( const portfolio::Folium& a ){
                                return a.name() == Constants::F_CENTROID_SPECTRUM; }) ) {
                        
                        if ( auto centroid = portfolio::get< adcontrols::MassSpectrumPtr >( fcentroid ) ) {
                            draw2( centroid );
                            pProcessedSpectrum_ = std::make_pair( fcentroid.id(), centroid );
                        } else {
                            pImpl_->processedSpectrum_->clear();
                        }

                        if ( auto fmethod = portfolio::find_first_of( fcentroid.attachments(), []( portfolio::Folium& a ){
                                    return portfolio::is_type< adcontrols::ProcessMethodPtr >( a ); }) ) {
                                
                            if ( auto method = portfolio::get< adcontrols::ProcessMethodPtr >( fmethod ) )
                                MainWindow::instance()->setProcessMethod( *method );
                        }
                        
                        if ( auto fpkinfo = portfolio::find_first_of( fcentroid.attachments(), []( portfolio::Folium& a ){
                                    return portfolio::is_type< adcontrols::MSPeakInfoPtr >( a ); } ) ) {
                            pkinfo_ = std::make_pair( fpkinfo.id(), portfolio::get< adcontrols::MSPeakInfoPtr >( fpkinfo ) );
                        }

                        if ( auto ftgt = portfolio::find_first_of( fcentroid.attachments(), []( portfolio::Folium& a ){
                                    return portfolio::is_type< adcontrols::TargetingPtr >( a ); } ) ) {
                            targeting_ = std::make_pair( ftgt.id(), portfolio::get< adcontrols::TargetingPtr >( ftgt ) );

                            // set corresponding targeting method to UI
                            if ( auto fmth = portfolio::find_first_of(
                                     ftgt.attachments(), [] ( portfolio::Folium& a ){
                                         return portfolio::is_type< adcontrols::ProcessMethodPtr >( a ); } ) ) {
                                if ( auto mth = portfolio::get< adcontrols::ProcessMethodPtr >( fmth ) )
                                    MainWindow::instance()->setProcessMethod( *mth );
                            }
                        }

                    } else {
                        pImpl_->processedSpectrum_->clear();
                    }

                    if ( auto f = portfolio::find_first_of( folium.attachments(), []( const portfolio::Folium& a ){
                                return a.name() == Constants::F_DFT_FILTERD; }) ) {
                        if ( auto ptr = portfolio::get< adcontrols::MassSpectrumPtr >( f ) ) {
                            // overlay DFT low pass filterd
                            draw2( ptr );
                        }
                    }
                    
                }
            }
        }
        else if ( folder.name() == L"Chromatograms" ) {
            pImpl_->ticPlot_->clear();
            if ( portfolio::is_type< adcontrols::ChromatogramPtr >( folium ) ) {
                if ( auto ptr = portfolio::get< adcontrols::ChromatogramPtr > ( folium ) ) {
                    draw( ptr, ptr->fcn() );
                    idActiveFolium_ = folium.id();
                    idChromatogramFolium( folium.id() );
                    if ( auto f = portfolio::find_first_of( folium.attachments(), []( portfolio::Folium& a ){
                                return portfolio::is_type< adcontrols::PeakResultPtr >( a ); }) ) {
                        auto pkresults = portfolio::get< adcontrols::PeakResultPtr >( f );
                        draw( pkresults );
                    }
                }
                // redraw all chromatograms with check marked
                auto folio = folder.folio();
                int idx = 0;
                for ( auto& folium: folio ) {
                    if ( folium.attribute( L"isChecked" ) == L"true" ) {
						processor->fetch(folium);
                        if ( auto cptr = portfolio::get< adcontrols::ChromatogramPtr >( folium ) ) {
                            pImpl_->setCheckedChromatogram( cptr, idx );
                            pImpl_->ticPlot_->setData( cptr, cptr->fcn() );
                        }
                    }
                    ++idx;
                }
            }
        }
    }
}

void
MSProcessingWnd::handleAxisChanged( adcontrols::hor_axis axis )
{
    using adplot::SpectrumWidget;

    axis_ = axis;
    pImpl_->set_time_axis( axis == adcontrols::hor_axis_mass ? false : true );
    auto plot_axis = ( axis == adcontrols::hor_axis_mass ? SpectrumWidget::HorizontalAxisMass : SpectrumWidget::HorizontalAxisTime );

    std::shared_ptr< adcontrols::MassSpectrometer > spectrometer;
    
    if ( auto processor = SessionManager::instance()->getActiveDataprocessor() )
        spectrometer = processor->massSpectrometer();

    pImpl_->processedSpectrum_->setAxis( plot_axis, true );

    pImpl_->profileSpectrum_->setAxis( plot_axis, true, [&](const QRectF& z, const adcontrols::MassSpectrum& ms, adplot::SpectrumWidget::HorizontalAxis axis ){
            if ( axis == adplot::SpectrumWidget::HorizontalAxisMass ) { // mass --> time
                auto range = spectrometer->timeFromMass( std::make_pair( z.left(), z.right() ), ms );
                return QRectF( range.first * std::micro::den, z.bottom(), ( range.second - range.first ) * std::micro::den, z.height() );
            } else { // time --> mass
                auto range = spectrometer->massFromTime( std::make_pair( z.left() / std::micro::den, z.right() / std::micro::den ), ms );
                return QRectF( range.first, z.bottom(), range.second - range.first, z.height() );
            }
        });
}

void
MSProcessingWnd::handleApplyMethod( const adcontrols::ProcessMethod& )
{
}

void
MSProcessingWnd::handleCustomMenuOnProcessedSpectrum( const QPoint& )
{
	// This is conflicting with picker's action, so it has moved to range selection slots
}

void
MSProcessingWnd::handleCurrentChanged( int idx, int fcn )
{
    if ( auto pkinfo = pkinfo_.second.lock() ) {

        if ( pkinfo->numSegments() > 1 )
            pImpl_->focusedFcn( fcn );

        adcontrols::segment_wrapper< const adcontrols::MSPeakInfo > fpks( *pkinfo );
        if ( fpks[ fcn ].size() > idx ) {
            auto pk = fpks[ fcn ].begin() + idx;
            pImpl_->currentChanged( *pk );
        }

    } else if ( auto ms = pProcessedSpectrum_.second.lock() ) {

        if ( ms->numSegments() > 1 )
            pImpl_->focusedFcn( fcn );

        adcontrols::segment_wrapper< const adcontrols::MassSpectrum > segs( *ms );
        if ( segs.size() > unsigned( fcn ) ) {
            pImpl_->currentChanged( segs[ fcn ], idx );
        }
    }
}

void
MSProcessingWnd::handleFormulaChanged( int idx, int fcn )
{
	pImpl_->processedSpectrum_->update_annotation();
    if ( Dataprocessor * dp = SessionManager::instance()->getActiveDataprocessor() )
        dp->formulaChanged(); // this makes processor dirty (setModified())

    emit dataChanged( QString::fromStdWString( pProfileSpectrum_.first ), QString::fromStdWString( pProcessedSpectrum_.first ), idx, fcn );
}

void
MSProcessingWnd::handleScanLawEst( const QVector< QPair<int, int> >& refs )
{
    if ( auto ms = pProcessedSpectrum_.second.lock() ) {

        if ( auto dp = SessionManager::instance()->getActiveDataprocessor() ) {
            std::vector< std::pair< int, int> > crefs( refs.size() );
            std::transform( refs.begin(), refs.end(), crefs.begin(), []( const auto& a ){ return std::make_pair( a.first, a.second ); } );
            if ( dp->estimateScanLaw( ms, crefs ) )
                return;
        }
    }

    estimateScanLaw( adcontrols::iids::adspectrometer_uuid );
}

void
MSProcessingWnd::estimateScanLaw( const boost::uuids::uuid& iid_spectrometer )
{
    // this will be relocate into 'CalibScanLaw' class defined in adtofprocessor
    adwidgets::ScanLawDialog2 dlg;
        
    if ( auto ms = pProcessedSpectrum_.second.lock() ) {
        
        for ( auto& fms: adcontrols::segment_wrapper< const adcontrols::MassSpectrum >( *ms ) ) {
            int mode = fms.getMSProperty().mode();
            for ( const auto& a: fms.get_annotations() ) {
                if ( a.dataFormat() == adcontrols::annotation::dataFormula && a.index() >= 0 ) {
                    dlg.addPeak( a.index()
                                 , QString::fromStdString( a.text() )
                                 , fms.getTime( a.index() )    // observed time-of-flight
                                 , fms.getMass( a.index() )    // matched mass
                                 , mode );
                }
            }
        }

        dlg.commit();
    }

    if ( Dataprocessor * dp = SessionManager::instance()->getActiveDataprocessor() ) {
        
        if ( auto db = dp->db() ) { // sqlite shared_ptr
            {
                adfs::stmt sql( *db );
                sql.prepare( "SELECT id,description,fLength FROM Spectrometer" );
                bool found(false);
                if ( sql.step() == adfs::sqlite_row ) {
                    dlg.setSpectrometerData( sql.get_column_value< boost::uuids::uuid >( 0 )
                                             , QString::fromStdWString( sql.get_column_value< std::wstring >( 1 ) )
                                             , sql.get_column_value< double >( 2 ) ); // fLength
                } else {
                    dlg.setSpectrometerData( iid_spectrometer, "", 0 );
                }
            }
            {
                adfs::stmt sql( *db );
                sql.prepare( "SELECT objuuid,objtext,acclVoltage,tDelay FROM ScanLaw" );
                while( sql.step() == adfs::sqlite_row ) {
                    dlg.addObserver( sql.get_column_value< boost::uuids::uuid >( 0 )
                                     , QString::fromStdString( sql.get_column_value< std::string >(1) )
                                     , sql.get_column_value< double >( 2 )
                                     , sql.get_column_value< double >( 3 )
                                     , dlg.peakCount() > 0 ? true : false );
                }
            }
        }
        
        if ( dlg.exec() != QDialog::Accepted )
            return;
        
        double t0 = dlg.tDelay() / std::micro::den;
        double acclV = dlg.acceleratorVoltage();
        adportable::TimeSquaredScanLaw law( acclV, t0, dlg.length() ); // <- todo: get 'L' from Spectrometer on db

        // update database
        auto list = dlg.checkedObservers();
        for ( auto& obj: list ) {
            adfs::stmt sql( *(dp->db()) );
            sql.prepare( "UPDATE ScanLaw SET acclVoltage=?,tDelay=? WHERE objtext=?" );
            sql.bind( 1 ) = acclV;
            sql.bind( 2 ) = t0;
            sql.bind( 3 ) = obj.toStdString();
            while ( sql.step() == adfs::sqlite_row )
                ;
        }        

        // assign masses for processed peak
        if ( auto ms = pProcessedSpectrum_.second.lock() ) {
            for ( auto& fms: adcontrols::segment_wrapper< adcontrols::MassSpectrum >( *ms ) ) {
                fms.getMSProperty().setAcceleratorVoltage( acclV );
                fms.getMSProperty().setTDelay( t0 );
                fms.assign_masses( [&]( double time, int mode ){ return law.getMass( time, mode ); } );
            }
        }
        
        // assign masses for profile spectrum
        if ( auto ms = pProfileSpectrum_.second.lock() ) {
            for ( auto& fms: adcontrols::segment_wrapper< adcontrols::MassSpectrum >( *ms ) ) {
                fms.getMSProperty().setAcceleratorVoltage( acclV );
                fms.getMSProperty().setTDelay( t0 );
                fms.assign_masses( [&]( double time, int mode ){ return law.getMass( time, mode ); } );
            }
        }
        handleDataMayChanged();   
    }
}

void
MSProcessingWnd::handleLockMass( const QVector< QPair<int, int> >& refs )
{
    if ( auto ms = pProcessedSpectrum_.second.lock() ) {

        adcontrols::lockmass::mslock lockmass;
        
        for ( auto ref : refs ) {
            adcontrols::lockmass::mslock::findReferences( lockmass, *ms, ref.first, ref.second );
        }

        if ( lockmass.fit() ) {
            if ( lockmass( *ms ) ) {
                ms->addDescription( adcontrols::description( L"Process", L"Mass locked" ) );
                pImpl_->processedSpectrum_->setZoomBase( ms->getAcquisitionMassRange() );
                pImpl_->processedSpectrum_->update_annotation();

                if ( Dataprocessor * dp = SessionManager::instance()->getActiveDataprocessor() )
					dp->lockMassHandled( idSpectrumFolium_, ms, lockmass ); // update profile, attribute data

                MainWindow::instance()->lockMassHandled( ms ); // update MSPeakTable
                handleDataMayChanged();

                emit dataChanged( QString::fromStdWString( idSpectrumFolium_ ), QString(), -1, -1 );
            }
        }
    }
}

void
MSProcessingWnd::handleDataMayChanged()
{
    pImpl_->profileSpectrum_->update();
    pImpl_->processedSpectrum_->update();
}

void
MSProcessingWnd::handleFoliumDataChanged( const QString& id )
{
    if ( id == QString::fromStdWString( idSpectrumFolium_ ) ) {
        pImpl_->profileSpectrum_->replot();
        pImpl_->processedSpectrum_->replot();
    }
}

void
MSProcessingWnd::handleCheckStateChanged( Dataprocessor* processor, portfolio::Folium& folium, bool isChecked )
{
    (void)processor;
    (void)isChecked;

    portfolio::Folder folder = folium.parentFolder();
	if ( !folder )
		return;
    if ( folder.name() == L"Chromatograms" ) {
        pImpl_->clearChromatograms();
        pImpl_->ticPlot_->clear();
        auto folio = folder.folio();
        int idx = 0;
        for ( auto& folium: folio ) {
            if ( folium.attribute( L"isChecked" ) == L"true" ) {
                if ( folium.empty() )
                    processor->fetch( folium );
                if ( auto cptr = portfolio::get< adcontrols::ChromatogramPtr >( folium ) ) 
					pImpl_->setCheckedChromatogram( cptr, idx );
                //pImpl_->ticPlot_->setData( cptr, idx );
            }
            ++idx;
        }
    }
}

void
MSProcessingWnd::handleRescaleY( int proto )
{
    pImpl_->profileSpectrum_->rescaleY( proto );
    pImpl_->processedSpectrum_->rescaleY( proto );
}

void
MSProcessingWnd::selectedOnChromatogram( const QRectF& rect )
{
	double x0 = pImpl_->ticPlot_->transform( QwtPlot::xBottom, rect.left() );
	double x1 = pImpl_->ticPlot_->transform( QwtPlot::xBottom, rect.right() );

    QMenu menu; 

	if ( int( std::abs( x1 - x0 ) ) <= 2 ) {
        
        if ( auto dp = SessionManager::instance()->getActiveDataprocessor() ) {
            if ( auto rawfile = dp->rawdata() ) {
                //--- v3 support -->
                if ( rawfile->dataformat_version() >= 3 ) {
                    // v3 data
                    auto readers = rawfile->dataReaders();
                    for ( auto& reader : readers ) {
                        if ( auto it = reader->findPos( rect.left() ) )
                            menu.addAction( QString::fromStdString(
                                                ( boost::format( "Select spectrum (%s) @ %.3lfs" ) % reader->display_name() % rect.left() ).str() )
                                            , [=] () { document::instance()->onSelectSpectrum_v3( rect.left(), it ); } );
                    }
                    
                } else {
                    // v2 data
                    size_t pos;
                    int index, rep, fcn;
                    double seconds;
                    if ( ! pImpl_->ticFinder2( rawfile, rect.left(), pos, index, rep, fcn, seconds ) ) {
                        seconds = 0;
                        index = fcn = -1;
                    }
                    menu.addAction( QString::fromStdString( 
                                        (boost::format( "Select a part of spectrum @%.3fs (%d/%d)" ) % seconds % index % fcn ).str() )
                                    , [=] () { document::instance()->onSelectSpectrum_v2( seconds, pos, fcn ); } );
                    
                    if ( index < 0 || fcn < 0 )
                        menu.actions().back()->setEnabled( false );
                    
                    menu.addAction( QString::fromStdString(
                                        ( boost::format( "Select a spectrum @%.3f min" ) % rect.left() ).str() )
                                    , [&] () { document::instance()->handleSelectTimeRangeOnChromatogram( rect.x(), rect.x() + rect.width() ); } );
                }
            }
        }
                    
        menu.addAction( tr("Copy image to clipboard"), [&] () { adplot::plot::copyToClipboard( pImpl_->ticPlot_ ); } );
        
        menu.addAction( tr( "Save SVG File" ), [&] () {
                QString name = QFileDialog::getSaveFileName( MainWindow::instance()
                                                             , "Save SVG File"
                                                             , MainWindow::makePrintFilename( idChromatogramFolium_, L"_" )
                                                             , tr( "SVG (*.svg)" ) );
                if ( ! name.isEmpty() )
                    adplot::plot::copyImageToFile( pImpl_->ticPlot_, name, "svg" );
            });

        menu.addAction( tr("Frequency analysis"), [&] () {
                if ( auto dp = SessionManager::instance()->getActiveDataprocessor() ) {
                    auto folium = dp->getPortfolio().findFolium( idChromatogramFolium_ );
                    if ( auto chr = portfolio::get< adcontrols::ChromatogramPtr >( folium ) ) {
                        power_spectrum( *chr );
                    }
                }
            } );
                
        menu.exec( QCursor::pos() );

    } else {
        document::instance()->handleSelectTimeRangeOnChromatogram( rect.x(), rect.x() + rect.width() );
    }

}

void
MSProcessingWnd::selectedOnProfile( const QRectF& rect )
{
	double x0 = pImpl_->profileSpectrum_->transform( QwtPlot::xBottom, rect.left() );
	double x1 = pImpl_->profileSpectrum_->transform( QwtPlot::xBottom, rect.right() );

    QMenu menu;
    
	if ( int( std::abs( x1 - x0 ) ) > 2 ) {

        using namespace adcontrols::metric;

        QString left = QString::number( rect.left(), 'f', 3 );
        QString right = QString::number( rect.right(), 'f', 3 );

        std::pair<size_t, size_t> range;
        
        if ( auto ms = pProfileSpectrum_.second.lock() ) {
            
            if ( ms->dataReaderUuid() != boost::uuids::uuid( {0} ) ) {
                // v3 data
                if ( Dataprocessor * dp = SessionManager::instance()->getActiveDataprocessor() ) {
                    if ( auto rd = dp->rawdata() ) {
                        if ( auto reader = rd->dataReader( ms->dataReaderUuid()) ) {
                            auto display_name = QString::fromStdString( reader->display_name() );
                            
                            // todo: chromatogram creation by m/z|time range
                            if ( axis_ == adcontrols::hor_axis_mass ) {
                                auto title = ( boost::format( "Make chromatogram from %s in m/z range %.3lf -- %.3lf" )
                                               % reader->display_name() % rect.left() % rect.right() ).str();
                                menu.addAction( QString::fromStdString (title.c_str() )
                                                , [=] () { make_chromatogram( reader, ms, axis_, rect.left(), rect.right() ); } );
                            } else {
                                auto title = ( boost::format( "Make chromatogram from %ss in range %.3lf -- %.3lf(us)" )
                                               % reader->display_name() % rect.left() % rect.right() ).str();
                                menu.addAction( QString::fromStdString( title.c_str() )
                                                , [=] () { make_chromatogram( reader, ms, axis_, rect.left() * 1.0e-6, rect.right() * 1.0e-6 ); } );
                            }
                        }
                    }
                }
            }
            
            if ( axis_ == adcontrols::hor_axis_time )
                range = std::make_pair( ms->getIndexFromTime( scale_to_base( rect.left(), micro ) )
                                        , ms->getIndexFromTime( scale_to_base( rect.right(), micro ) ) );
            else {
                const double * masses = ms->getMassArray();
                range = std::make_pair( std::distance( masses, std::lower_bound( masses, masses + ms->size(), rect.left() ) )
                                        , std::distance( masses, std::lower_bound( masses, masses + ms->size(), rect.right() ) ) );
            }
            
            const auto f_rms = ( axis_ == adcontrols::hor_axis_time ) ?
                tr("RMS in range %1 -- %2(us)") : tr("RMS in m/z range %1 -- %2");
            const auto f_maxval = ( axis_ == adcontrols::hor_axis_time ) ?
                tr("Max value in range %1 -- %2(us)") : tr("Max value in m/z range %1 -- %2");
            const auto f_count = ( axis_ == adcontrols::hor_axis_time ) ?
                tr("Count/Area in range %1 -- %2(us)") : tr("Count/Area in m/z range %1 -- %2");

            menu.addAction( tr( "y-zoom" )
                , [=](){ pImpl_->profileSpectrum_->yZoom( rect.left(), rect.right() ); } );
            
            menu.addAction( QString( f_rms ).arg( left, right ), [&](){
                    if ( compute_rms( rect.left(), rect.right() ) > 0 )
                        draw1();                        
                });
            
            menu.addAction( QString( f_maxval ).arg( left, right ), [&](){
                    compute_minmax( rect.left(), rect.right() );
                    draw1();                        
                });

            menu.addAction( QString( f_count ).arg( left, right ), [&](){
                    compute_count( rect.left(), rect.right() );
                    draw1();
                });
            
            menu.addAction( QString( tr("Frequency analysis") ).arg( left, right ), [&](){
                    power_spectrum( *ms, range );
                });
        }

        menu.exec( QCursor::pos() );
        
	} else {
        using adplot::plot;

        bool isHistogram( false );
        if ( auto ms = pProfileSpectrum_.second.lock() )
            isHistogram = ms->isCentroid();
        
        menu.addAction( tr( "Correct baseline" ),   [this] () { correct_baseline(); draw1(); } );
        menu.addAction( tr( "Copy to clipboard" ),  [this] () { plot::copyToClipboard( pImpl_->profileSpectrum_ ); } );
        menu.addAction( tr( "Frequency analysis" ), [this] () { frequency_analysis(); } );
        menu.addAction( tr( "Zero filling" ),       [this] () { zero_filling(); } );
        menu.addAction( tr( "Save image file..." ), [this] () { save_image_file(); } );
        menu.addAction( tr( "Auto Y Scale" ),       [this] () { autoYScale( pImpl_->profileSpectrum_ ); } );
        menu.actions()[4]->setCheckable( true );
        menu.actions()[4]->setChecked( pImpl_->profileSpectrum_->zoomer()->autoYScale() );

        if ( isHistogram )
            menu.actions()[2]->setEnabled( false ); // Frequency analysis

        auto iid_spectrometers = adcontrols::MassSpectrometerBroker::installed_uuids();

        for ( auto model : iid_spectrometers ) {
            menu.addAction( QString( tr( "Re-assign masses using %1 scan law" ) ).arg( QString::fromStdString( model.second ) )
                            , [&](){
                                assign_masses_to_profile( model );
                                pImpl_->profileSpectrum_->replot();
                            });
        }

        menu.exec( QCursor::pos() );
    }
}

void
MSProcessingWnd::selectedOnPowerPlot( const QRectF& rect )
{
    QMenu menu;

    menu.addAction( tr( "Copy to Clipboard" ), [&](){ adplot::plot::copyToClipboard( pImpl_->pwplot_ ); } );

    menu.addAction( tr( "Save as SVG File..." ), [&](){
            QString name =
                QFileDialog::getSaveFileName( MainWindow::instance(), "Save SVG File"
                                              , MainWindow::makePrintFilename( idSpectrumFolium_, L"_power_" ), tr( "SVG (*.svg)" ) );
            if ( !name.isEmpty() )
                adplot::plot::copyImageToFile( pImpl_->pwplot_, name, "svg" );
        } );

    menu.addAction( tr( "Dismiss" ), [&](){ pImpl_->pwplot_->hide(); } );

    menu.exec( QCursor::pos() );    
}

void
MSProcessingWnd::selectedOnProcessed( const QRectF& rect )
{
	double x0 = pImpl_->profileSpectrum_->transform( QwtPlot::xBottom, rect.left() );
	double x1 = pImpl_->profileSpectrum_->transform( QwtPlot::xBottom, rect.right() );

    QMenu menu;

    menu.addAction( tr( "y-zoom" ), [&](){ pImpl_->processedSpectrum_->yZoom( rect.left(), rect.right() ); } );
    
    menu.addAction( tr( "Make mass chromatograms" )
                    , [&]{ make_chromatograms_from_peaks( pProcessedSpectrum_.second.lock(), axis_, rect.left(), rect.right() ); } );

    bool hasRange = int( std::abs( x1 - x0 ) ) > 2;
    auto ptr = pProcessedSpectrum_.second.lock();
    auto actions = menu.actions();
    if ( actions.size() >= 2 ) {
        actions[ 0 ]->setEnabled( hasRange );
        actions[ 1 ]->setEnabled( hasRange && ptr && ptr->isCentroid() );
    }
    
    menu.addAction( tr( "Copy to clipboard" ), [&]{ adplot::plot::copyToClipboard( pImpl_->processedSpectrum_ ); } );

    menu.addAction( tr( "Save as SVG File..." ), [&]{
            QString name = QFileDialog::getSaveFileName( MainWindow::instance(), "Save SVG File"
                                                         , MainWindow::makePrintFilename( idSpectrumFolium_, L"_processed_" )
                                                         , tr("SVG (*.svg)") );
            if ( ! name.isEmpty() )
                adplot::plot::copyImageToFile( pImpl_->profileSpectrum_, name, "svg" );                
        });
    
    if ( auto dp = SessionManager::instance()->getActiveDataprocessor() ) {
        dp->addContextMenu( adprocessor::ContextMenuOnProcessedMS, menu, ptr
                            , { rect.left(), rect.right() }, axis_ == adcontrols::hor_axis_time );
    }
    
    menu.exec( QCursor::pos() );
}

void
MSProcessingWnd::handlePrintCurrentView( const QString& pdfname )
{
	// A4 := 210mm x 297mm (8.27 x 11.69 inch)
    QSizeF sizeMM( 180, 80 );

    int resolution = 85;
	const double mmToInch = 1.0 / 25.4;
    const QSizeF size = sizeMM * mmToInch * resolution;

	QPrinter printer;
    printer.setColorMode( QPrinter::Color );
    printer.setPaperSize( QPrinter::A4 );
    printer.setFullPage( false );
    
	portfolio::Folium folium;
    printer.setDocName( "QtPlatz Process Report" );
	if ( Dataprocessor * dp = SessionManager::instance()->getActiveDataprocessor() ) {
        folium = dp->getPortfolio().findFolium( idActiveFolium_ );
    }
    printer.setOutputFileName( pdfname );
    // printer.setOutputFormat( QPrinter::PdfFormat );
    printer.setResolution( resolution );

    QPainter painter( &printer );

	QRectF boundingRect;
	QRectF drawRect( 0.0, 0.0, printer.width(), (12.0/72)*printer.resolution() );

	painter.drawText( drawRect, Qt::TextWordWrap, folium.fullpath().c_str(), &boundingRect );
	
    QwtPlotRenderer renderer;
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasBackground, true );
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasFrame, true );
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardBackground, true );

	drawRect.setTop( boundingRect.bottom() );
	drawRect.setHeight( size.height() );
	drawRect.setWidth( size.width() );
	renderer.render( pImpl_->processedSpectrum_, &painter, drawRect );

	drawRect.setTop( drawRect.bottom() );
	drawRect.setHeight( size.height() );
    renderer.render( pImpl_->profileSpectrum_, &painter, drawRect );

    // -- starting method print
    drawRect.setTop( drawRect.bottom() + 0.5 * resolution );
    drawRect.setHeight( printer.height() - drawRect.top() );
    
    {
        portfolio::Folio attachments = folium.attachments();
        auto it = portfolio::Folium::find<adcontrols::MassSpectrumPtr>( attachments.begin(), attachments.end() );
        if ( it != attachments.end() ) {
            auto atts = it->attachments();
            auto pf = portfolio::Folium::find< adcontrols::ProcessMethodPtr >( atts.begin(), atts.end() );
            if ( pf != atts.end() ) {
                adcontrols::ProcessMethodPtr pm;
                if ( portfolio::Folium::get< adcontrols::ProcessMethodPtr >( pm, *pf ) ) {
                    std::wostringstream o;
                    adcontrols::ProcessMethod::xml_archive( o, *pm );
                    pugi::xml_document dom;
                    auto result = dom.load( pugi::as_utf8( o.str() ).c_str() );
                    if ( result )
                        adpublisher::printer::print( printer, painter, drawRect, dom, "process-method-html.xsl" );
                }
            }
        }
    }

    ///////////
    if ( auto p = MainWindow::instance()->findChild< adwidgets::MSPeakTable * >( "MSPeakTable" ) ) {
        p->handlePrint( printer, painter );
    }
}

bool
MSProcessingWnd::assign_masses_to_profile( const std::pair< boost::uuids::uuid, std::string >& iid_spectrometer )
{
    if ( iid_spectrometer.first == adcontrols::iids::adspectrometer_uuid ) {
        estimateScanLaw( iid_spectrometer.first );
        return true;
    } else {
        estimateScanLaw( iid_spectrometer.first );
        return true;        
    }
    return false;
}

bool
MSProcessingWnd::assign_masses_to_profile()
{
	adportable::TimeSquaredScanLaw law;

    std::pair< double, double > mass_range;
    
    if ( auto x = this->pProfileSpectrum_.second.lock() ) {
        
        adcontrols::segment_wrapper< adcontrols::MassSpectrum > segments( *x );
        
        for ( auto& ms: segments ) {
            for ( size_t idx = 0; idx < ms.size(); ++idx ) {
                double m = law.getMass( ms.getTime( idx ), 0 );
                ms.setMass( idx, m );
                if ( idx == 0 )
                    mass_range.first = std::min( mass_range.first, m );
                if ( idx == ms.size() - 1 )
                    mass_range.second = std::max( mass_range.second, m );
            }
        }
        x->setAcquisitionMassRange( mass_range.first, mass_range.second );

        adcontrols::MSProperty prop( x->getMSProperty() );
		prop.setAcceleratorVoltage( law.kAcceleratorVoltage() );
		prop.setTDelay( law.tDelay() );
		x->setMSProperty( prop );

        return true;
    }
    return false;
}


double
MSProcessingWnd::correct_baseline()
{
    double tic = 0;

    if ( auto x = this->pProfileSpectrum_.second.lock() ) {

        std::wostringstream o;
        o << L"Baseline corrected";

		adcontrols::segment_wrapper< adcontrols::MassSpectrum > segments( *x );

		for ( auto& ms: segments ) {

            double dbase(0), rms(0);
            const double * data = ms.getIntensityArray();
            tic += adportable::spectrum_processor::tic( static_cast< unsigned int >( ms.size() ), data, dbase, rms );
            for ( size_t idx = 0; idx < ms.size(); ++idx )
                ms.setIntensity( idx, data[ idx ] - dbase );
            double h = *std::max_element( ms.getIntensityArray(), ms.getIntensityArray() + ms.size() );
            o << boost::wformat( L" H=%.2f/RMS=%.2f" ) % h % rms;
		}
		x->addDescription( adcontrols::description( L"process", o.str() ) );
	}
	return tic;
}

double
MSProcessingWnd::compute_rms( double s, double e )
{
	if ( auto ptr = this->pProfileSpectrum_.second.lock() ) {

        namespace pfx = adcontrols::metric;

		adcontrols::segment_wrapper< adcontrols::MassSpectrum > segments( *ptr );
        
        for ( auto& ms: segments ) {

            std::pair< size_t, size_t > range;
            if ( pImpl_->is_time_axis_ ) {
                range.first = ms.getIndexFromTime( scale_to_base(s, pfx::micro), false );
                range.second = ms.getIndexFromTime( scale_to_base(e, pfx::micro), true );
            } else {
                const double * masses = ms.getMassArray();
                range.first = std::distance( masses, std::lower_bound( masses, masses + ms.size(), s ) );
                range.second = std::distance( masses, std::lower_bound( masses, masses + ms.size(), e ) );
            }
            size_t n = range.second - range.first + 1;
            if ( n >= 5 ) {

                adportable::array_wrapper<const double> data( ms.getIntensityArray() + range.first, n );

				double sum = std::accumulate( data.begin(), data.end(), 0.0 );
                double m = sum / data.size();
                double sdd = std::accumulate( data.begin(), data.end(), 0.0, [=]( double a, double x ){ return a + ( (x - m) * (x - m) ); }) / n;
                double rms = std::sqrt( sdd );

				using namespace adcontrols::metric;
                
                ptr->addDescription( adcontrols::description( L"process"
                                                              , (boost::wformat(L"RMS[%.3lf-%.3lf(&mu;s),N=%d]=%.3lf")
                                                                 % scale_to_micro( ms.getTime(range.first) )
                                                                 % scale_to_micro( ms.getTime(range.second) )
                                                                 % n
                                                                 % rms).str() ) );

                QString text = QString::fromStdString(
                    ( boost::format("rms(start,end,N,rms)\t%.14f\t%.14f\t%d\t%.7f")
                      % scale_to_micro( ms.getTime( range.first ) )
                      % scale_to_micro( ms.getTime( range.second ) )
                      % n
                      % rms).str() );

                QApplication::clipboard()->setText( text );

				return rms;
            }
        }

    }
	return 0;
}

std::pair< double, double >
MSProcessingWnd::compute_minmax( double s, double e )
{
    using namespace adcontrols::metric;

	if ( auto ptr = this->pProfileSpectrum_.second.lock() ) {

		adcontrols::segment_wrapper< adcontrols::MassSpectrum > segments( *ptr );
        
        for ( auto& ms: segments ) {

            std::pair< size_t, size_t > range;
            if ( pImpl_->is_time_axis_ ) {
                range.first = ms.getIndexFromTime( scale_to_base( s, micro ), false );
                range.second = ms.getIndexFromTime( scale_to_base( e, micro), true );
            } else {
                const double * masses = ms.getMassArray();
                range.first = std::distance( masses, std::lower_bound( masses, masses + ms.size(), s ) );
                range.second = std::distance( masses, std::lower_bound( masses, masses + ms.size(), e ) );
            }

            size_t n = range.second - range.first + 1;
            if ( n >= 5 ) {

                adportable::array_wrapper<const double> data( ms.getIntensityArray(), ms.size() );
                auto pair = std::minmax_element( data.begin() + range.first, data.begin() + range.second );

                std::pair<double, double> result = std::make_pair( *pair.first, *pair.second );
                std::pair< size_t, size_t > index = std::make_pair( std::distance( data.begin(), pair.first ), std::distance( data.begin(), pair.second ) );
                
                std::pair< double, double > time = std::make_pair(
                    adcontrols::MSProperty::toSeconds( index.first, ms.getMSProperty().samplingInfo() )
                    , adcontrols::MSProperty::toSeconds( index.second, ms.getMSProperty().samplingInfo() )
                    );
                
                ptr->addDescription( adcontrols::description( L"process"
                                                              , ( boost::wformat(L"min at %.4us=%.3f; max at %.4us=%3f" )
                                                                  % scale_to_micro(time.first) % result.first
                                                                  % scale_to_micro(time.second) % result.second ).str() ) );

                std::ostringstream o;
                o << boost::format("min @\t%d\t%.14lf\t%.7f") % index.first % scale_to_micro(time.first) % result.first
                  << boost::format("\tmax @\t%d\t%.14lf\t%.7f") % index.second % scale_to_micro(time.second) % result.second;

                QApplication::clipboard()->setText( QString::fromStdString( o.str() ) );

				return result;
            }
        }

    }
	return std::make_pair(0,0);
}

double
MSProcessingWnd::compute_count( double s, double e )
{
	if ( auto ptr = pProfileSpectrum_.second.lock() ) {
        
        using namespace adcontrols::metric;

        QString clipboard;

		adcontrols::segment_wrapper< adcontrols::MassSpectrum > segments( *ptr );

        int idx = 0;
        
        for ( auto& ms: segments ) {

            const auto& prop = ms.getMSProperty();

            bool found( false );
            std::pair< size_t, size_t > range = { 0, 0 };

            if ( pImpl_->is_time_axis_ ) {

                s = scale_to_base( s, micro );
                e = scale_to_base( e, micro );
                if ( ms.getTime( 0 ) <= e && ms.getTime( ms.size() - 1 ) >= s ) {
                	if ( const double * times = ms.getTimeArray() ) {
                		range.first = std::distance( times, std::lower_bound( times, times + ms.size(), s));
                		range.second = std::distance( times, std::lower_bound( times, times + ms.size(), e));
                		found = true;
                	}
                }

            } else {
            	if ( ms.getMass( 0 ) <= e && ms.getMass( ms.size() - 1 ) >= s ) {
            		if ( const double * masses = ms.getMassArray() ) {
            			range.first = std::distance( masses, std::lower_bound( masses, masses + ms.size(), s ) );
            			range.second = std::distance( masses, std::lower_bound( masses, masses + ms.size(), e ) );
            			found = true;
            		}
            	}
            }
            
            if ( found ) {

                // ADDEBUG() << "data range: " << range.first << ", " << range.second;
                
                const double * data = ms.getIntensityArray();
                double count = std::accumulate( data + range.first, data + range.second, 0.0 );
                
                auto maxIdx = std::distance( data, std::max_element( data + range.first, data + range.second ) );

                double apex = ( pImpl_->is_time_axis_ ) ? ms.getTime( maxIdx ) : ms.getMass( maxIdx );
                double height = ms.getIntensity( maxIdx );

                char fmt = ( pImpl_->is_time_axis_ ) ? 'e' : 'f';
                
                clipboard.append(
                    QString("#%1\tCount,height,apex(m/z|time),N,n-trig\t%2\t%3\t%4\t%5\t%6\n").arg(
                        QString::number( idx )
                	    , QString::number( count )
                        , QString::number( height )
                        , QString::number( apex, fmt, 7 )
                        , QString::number( range.second - range.first )
                        , QString::number( ms.getMSProperty().numAverage() )
                        )
                    );
            }
            ++idx;
        }
        QApplication::clipboard()->setText( clipboard );
    }
	return 0;
}

bool
MSProcessingWnd::power_spectrum( const adcontrols::MassSpectrum& ms
                                 , const std::pair< size_t, size_t >& range )
{
    const size_t size = range.second - range.first + 1;
    if ( size < 8 )
        return false;
    size_t n = 1;
    while ( size >> n )
        ++n;
    size_t N = 1LL << ( n - 1 );

    std::vector< std::complex< double > > spc( N ), fft( N );
	const double * intens = ms.getIntensityArray() + range.first;
	for ( size_t i = 0; i < N && i < size; ++i )
        spc[ i ] = std::complex< double >( intens[ i ] );

    adportable::fft::fourier_transform( fft, spc, false );

	const double T = N * ms.getMSProperty().samplingInfo().fSampInterval();
    std::vector< double > x( N / 2 ), y( N / 2 );
    for ( size_t i = 0; i < N / 2; ++i ) {
        y[i] = ( ( fft[ i ].real() * fft[ i ].real() ) + ( fft[ i ].imag() * fft[ i ].imag() ) ) / ( double(N) * N );
        x[i] = double( i ) / T * 1e-6; // MHz
    }
    double dc = fft[0].real();
    double nyquist = fft[ N / 2 ].real();

    //----- draw 
    std::ostringstream o;
    o << boost::format( "N=%d Power: DC=%.7g Nyquist=%.7g" ) % (x.size() * 2) % dc % nyquist;
    QString title = QString("[%1] %2").arg( MainWindow::makeDisplayName( idSpectrumFolium_ ), QString::fromStdString( o.str() ) );
    pImpl_->pwplot_->setData( x.size() - 1, x.data() + 1, y.data() + 1 );
    pImpl_->pwplot_->setTitle( title );
    pImpl_->pwplot_->show();
    pImpl_->pwplot_->xBottomTitle( "Frequency (MHz)" );
	return true;
}

void
MSProcessingWnd::power_spectrum( const adcontrols::Chromatogram& c )
{
    const size_t size = c.size();
    if ( size >= 8 ) {
        size_t n = 1;
        while ( size >> n )
            ++n;
        size_t N = 1 << ( n - 1 );

        std::vector< std::complex< double > > spc(N), fft(N);
        int idx( 0 );
        for ( auto it = c.begin(); it != c.end() && idx < N; ++it )
            spc[ idx++ ] = std::complex< double >( it.intensity() );

        double fInterval = c.sampInterval();
        
        adportable::fft::fourier_transform( fft, spc, false );
        
        const double T = N * fInterval;
        std::vector< double > x( N / 2 ), y( N / 2 );

        std::transform( fft.begin(), fft.begin() + N / 2, y.begin(), [&]( const std::complex<double>& d ){
                return ( ( d.real() * d.real() ) + ( d.imag() * d.imag() ) ) / ( double(N) * N ); } );

        //std::iota( x.begin(), x.end(), frequency(T) );
        for ( size_t i = 0; i < N / 2; ++i )
            x [ i ] = double( i ) / T; // Hz 

        double dc = fft[0].real();
        double nyquist = fft[ N / 2 ].real();

        std::ostringstream o;
        o << boost::format( "N=%d Power: DC=%.7g Nyquist=%.7g" ) % (x.size() * 2) % dc % nyquist;
        QString title = QString("[%1]&nbsp;&nbsp;&nbsp;&nbsp;%2").arg( MainWindow::makeDisplayName( idChromatogramFolium_ ), QString::fromStdString( o.str() ) );
        pImpl_->pwplot_->setData( x.size() - 1, x.data() + 1, y.data() + 1 ); // skip DC component from plot
        pImpl_->pwplot_->setTitle( title );
        pImpl_->pwplot_->show();
        pImpl_->pwplot_->xBottomTitle( "Frequency (Hz)" );
    }
}

void
MSProcessingWnd::frequency_analysis()
{
    if ( auto ms = pProfileSpectrum_.second.lock() ) {
        auto range = std::make_pair( size_t( 0 ), ms->size() - 1 );
        power_spectrum( *ms, range );
    }
}

void
MSProcessingWnd::zero_filling()
{
    if ( auto ms = pProfileSpectrum_.second.lock() ) {

        auto dp = SessionManager::instance()->getActiveDataprocessor();
        if ( auto spectrometer = dp ? dp->massSpectrometer() : nullptr ) {

            if ( auto law = spectrometer->scanLaw() ) {
                for ( auto& fms: adcontrols::segment_wrapper< adcontrols::MassSpectrum >( *ms ) ) {
                    using adcontrols::waveform_filter;
                    waveform_filter::fft4c::zero_filling( fms, 100.0e6, [&]( double t ){
                            return law->getMass( t, fms.mode() );
                        } );
                }
            }
        }
    }
}

// process on profile spectrum
void
MSProcessingWnd::make_chromatogram( const adcontrols::DataReader * reader
                                    , std::shared_ptr< const adcontrols::MassSpectrum > ms
                                    , adcontrols::hor_axis axis
                                    , double s, double e )
{
    if ( Dataprocessor * processor = SessionManager::instance()->getActiveDataprocessor() ) {
        auto pm = std::make_shared< adcontrols::ProcessMethod >();
        MainWindow::instance()->getProcessMethod( *pm );
        // [3]
        DataprocessWorker::instance()->createChromatogramByAxisRange3( processor, pm, axis, std::make_pair( s, e ), reader );
    }
}


/* make chromatograms from centroid data
 */
void
MSProcessingWnd::make_chromatograms_from_peaks( std::shared_ptr< const adcontrols::MassSpectrum > ptr
                                                , adcontrols::hor_axis axis
                                                , double left
                                                , double right )
{
    if ( ptr && ptr->isCentroid() ) {

        std::shared_ptr< adcontrols::MSPeakInfo > xpkinfo;

        if ( auto pkinfo = pkinfo_.second.lock() ) {

            for ( const auto& pkseg: adcontrols::segment_wrapper< const adcontrols::MSPeakInfo >( *pkinfo ) ) {

                adcontrols::MSPeakInfo xInfo;
            
                auto beg = std::lower_bound( pkseg.begin(), pkseg.end(), left, [&]( const adcontrols::MSPeakInfoItem& a, const double& left ) {
                        return ( axis == adcontrols::hor_axis_mass ) ? a.mass() < left : a.time() < left;  });

                if ( beg != pkseg.end() ) {
                    auto end = std::lower_bound( beg, pkseg.end(), right, [&]( const adcontrols::MSPeakInfoItem& a, const double& right ) {
                            return ( axis == adcontrols::hor_axis_mass ) ? right < a.mass() : right < a.time(); });

                    xInfo.setMode( pkseg.mode() );
                    xInfo.setProtocol( pkseg.protocolId(), pkseg.nProtocols() );
                    std::for_each( beg, end, [&]( const adcontrols::MSPeakInfoItem& a ){ xInfo << a; } );
                    if ( ! xpkinfo )
                        xpkinfo = std::make_shared< adcontrols::MSPeakInfo >( xInfo );
                    else
                        xpkinfo->addSegment( xInfo );
                }
            }
        }
        
        if ( xpkinfo ) {
            

            if ( Dataprocessor * processor = SessionManager::instance()->getActiveDataprocessor() ) {
                if ( auto file = processor->rawdata() ) {
                    if ( file->dataformat_version() >= 3 ) {
                        if ( auto reader = file->dataReader( ptr->dataReaderUuid() ) ) {
                            auto pm = std::make_shared< adcontrols::ProcessMethod >();
                            MainWindow::instance()->getProcessMethod( *pm );
                            DataprocessWorker::instance()->createChromatogramsByPeakInfo3( processor, pm, axis, xpkinfo, reader );
                        }
                    } else {
                        // DataprocessWorker::instance()->createChromatograms( processor, axis_, ranges, ptr->dataReaderUuid() );
                    }
                }
            }
        }
    }
}

void
MSProcessingWnd::save_image_file()
{
    auto settings = document::instance()->settings();
    using namespace dataproc::Constants;
    settings->beginGroup( GRP_SPECTRUM_IMAGE );
    QString fmt = settings->value( KEY_IMAGEE_FORMAT, "svg" ).toString();
    bool compress = settings->value( KEY_COMPRESS, true ).toBool();
    int dpi = settings->value( KEY_DPI, 300 ).toInt();
    settings->endGroup();
            
    std::string dfmt = "." + fmt.toStdString();
            
    adwidgets::FileDialog dlg( MainWindow::instance()
                               , tr( "Save Image File" )
                               , MainWindow::makePrintFilename( idSpectrumFolium_, L"_profile_", dfmt.c_str() ) );
            
    dlg.setVectorCompression( tr( "Compress vector graphics" ), compress, fmt, dpi );
            
    if ( dlg.exec() == QDialog::Accepted ) {
        auto result = dlg.selectedFiles();
        boost::filesystem::path path( result.at( 0 ).toStdWString() );
        const char * format = "svg";
        if ( path.extension() == ".pdf" )
            format = "pdf";
                
        settings->beginGroup( GRP_SPECTRUM_IMAGE );
        settings->setValue( KEY_IMAGEE_FORMAT, format );
        settings->setValue( KEY_COMPRESS, dlg.vectorCompression() );
        settings->setValue( KEY_DPI, dlg.dpi() );
        settings->endGroup();
                
        adplot::plot::copyImageToFile( pImpl_->profileSpectrum_, result.at( 0 ), format, dlg.vectorCompression(), dlg.dpi() );
    }
}

void
MSProcessingWnd::autoYScale( adplot::plot * plot )
{
    if ( auto zoomer = plot->zoomer() ) 
        zoomer->autoYScale( ! zoomer->autoYScale() );
}

void
MSProcessingWnd::autoYZoom( adplot::plot * plot, double xmin, double xmax )
{
    
}

