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

#include "msprocessingwnd.hpp"
#include "dataprocplugin.hpp"
#include "dataprocessor.hpp"
#include "dataprocessworker.hpp"
#include "sessionmanager.hpp"
#include "mainwindow.hpp"
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/datafile.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/descriptions.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/lockmass.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/mspeakinfoitem.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/peakresult.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/targeting.hpp>
#include <adcontrols/waveform.hpp>
#include <adlog/logger.hpp>
#include <adportable/scoped_debug.hpp>
#include <adportable/spectrum_processor.hpp>
#include <adportable/array_wrapper.hpp>
#include <adportable/xml_serializer.hpp>
#include <adportable/fft.hpp>
#include <adportable/timesquaredscanlaw.hpp>
#include <adutils/processeddata.hpp>
#include <adwplot/picker.hpp>
#include <adwplot/peakmarker.hpp>
#include <adwplot/chromatogramwidget.hpp>
#include <adwplot/spectrumwidget.hpp>
#include <adwplot/tracewidget.hpp>
#include <portfolio/portfolio.hpp>
#include <portfolio/folium.hpp>
#include <portfolio/folder.hpp>
#include <qtwrapper/xmlformatter.hpp>
#include <qtwrapper/font.hpp>
#include <coreplugin/minisplitter.h>
#include <qwt_scale_widget.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_renderer.h>
#include <qapplication.h>
#include <qsvggenerator.h>
#include <qprinter.h>
#include <QBoxLayout>
#include <QMenu>
#include <qclipboard.h>
#include <QFileDialog>
#include <boost/variant.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/exception/all.hpp>
#include "selchanged.hpp"
#include <sstream>
#include <array>
#include <numeric>
#include <complex>

using namespace dataproc;

namespace dataproc {

    class MSProcessingWndImpl {
    public:
        MSProcessingWndImpl() : ticPlot_(0)
                              , profileSpectrum_(0)
                              , processedSpectrum_(0)
                              , pwplot_( new adwplot::TraceWidget )
                              , is_time_axis_( false ){
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

        adwplot::ChromatogramWidget * ticPlot_;
        adwplot::SpectrumWidget * profileSpectrum_;
        adwplot::SpectrumWidget * processedSpectrum_;
        adwplot::TraceWidget * pwplot_;
        std::shared_ptr< adwplot::PeakMarker > profile_marker_;
        std::shared_ptr< adwplot::PeakMarker > processed_marker_;
        bool is_time_axis_;
    };

}

MSProcessingWnd::MSProcessingWnd(QWidget *parent) : QWidget(parent)
{
    init();
}

void
MSProcessingWnd::init()
{
    pImpl_ = std::make_shared< MSProcessingWndImpl >();

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
            pImpl_->profile_marker_ = std::make_shared< adwplot::PeakMarker >();
            pImpl_->profile_marker_->attach( pImpl_->profileSpectrum_ );
            pImpl_->profile_marker_->visible( true );
            pImpl_->profile_marker_->setYAxis( QwtPlot::yLeft );
        }

        if ( ( pImpl_->processedSpectrum_ = new adwplot::SpectrumWidget(this) ) ) {
            pImpl_->processedSpectrum_->setMinimumHeight( 80 );
			connect( pImpl_->processedSpectrum_, SIGNAL( onSelected( const QPointF& ) ), this, SLOT( selectedOnProcessed( const QPointF& ) ) );
			connect( pImpl_->processedSpectrum_, SIGNAL( onSelected( const QRectF& ) ), this, SLOT( selectedOnProcessed( const QRectF& ) ) );
            adwplot::Zoomer * zoomer = &pImpl_->processedSpectrum_->zoomer();
            connect( zoomer, SIGNAL( zoomed( const QRectF& ) ), this, SLOT( handleZoomedOnSpectrum( const QRectF& ) ) );

            pImpl_->processed_marker_ = std::make_shared< adwplot::PeakMarker >();
            pImpl_->processed_marker_->attach( pImpl_->processedSpectrum_ );
            pImpl_->processed_marker_->visible( true );
            pImpl_->processed_marker_->setYAxis( QwtPlot::yLeft );
        }
 
		pImpl_->ticPlot_->axisWidget( QwtPlot::yLeft )->scaleDraw()->setMinimumExtent( 80 );
		pImpl_->profileSpectrum_->axisWidget( QwtPlot::yLeft )->scaleDraw()->setMinimumExtent( 80 );
		pImpl_->processedSpectrum_->axisWidget( QwtPlot::yLeft )->scaleDraw()->setMinimumExtent( 80 );

        pImpl_->pwplot_->setMinimumHeight( 80 );
		pImpl_->pwplot_->axisWidget( QwtPlot::yLeft )->scaleDraw()->setMinimumExtent( 80 );
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
MSProcessingWnd::draw_profile( const std::wstring& guid, adutils::MassSpectrumPtr& ptr )
{
    pProfileSpectrum_ = std::make_pair( guid, ptr );
    pImpl_->profileSpectrum_->setData( ptr, static_cast<int>(drawIdx1_++) );
    QString title = QString("[%1]&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;").arg( MainWindow::makeDisplayName( idSpectrumFolium_ ) );
	for ( auto text: ptr->getDescriptions() )
		title += QString::fromStdWString( text.text() + L", " );
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
            title += QString::fromStdWString( text.text() + L", " );

        pImpl_->profileSpectrum_->setTitle( title );
        pImpl_->processedSpectrum_->clear();
    }
}

void
MSProcessingWnd::draw2( adutils::MassSpectrumPtr& ptr )
{
    if ( ptr->isCentroid() )
        pImpl_->processedSpectrum_->setData( ptr, static_cast<int>(drawIdx2_++) );
    else
        pImpl_->processedSpectrum_->setData( ptr, static_cast<int>(drawIdx2_++), true );        
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
    const adcontrols::LCMSDataset * dset = processor->getLCMSDataset();
    portfolio::Portfolio portfolio = processor->getPortfolio();

    if ( dset ) {
		size_t nfcn = dset->getFunctionCount();

		portfolio::Folder folder = portfolio.findFolder( L"Chromatograms" );
		if ( folder.nil() )
			folder = processor->getPortfolio().addFolder( L"Chromatograms" );

		for ( size_t fcn = 0; fcn < nfcn; ++fcn ) {
            std::wstring title = ( boost::wformat( L"TIC.%1%" ) % (fcn + 1) ).str();
            
			portfolio::Folium folium = folder.findFoliumByName( std::wstring( L"TIC/" ) + title );
            if ( folium.nil() ) {   // add TIC if not yet added
				adcontrols::Chromatogram c;
                if ( dset->getTIC( static_cast<int>(fcn), c ) ) {
                    if ( c.isConstantSampledData() )
                        c.getTimeArray();
                    c.addDescription( adcontrols::Description( L"acquire.title", title ) );
                    adcontrols::ProcessMethod m;
                    MainWindow::instance()->getProcessMethod( m );
                    folium = processor->addChromatogram( c, m );
				}
            }
            if ( folium.attribute(L"protoId").empty() )
                folium.setAttribute( L"protoId", (boost::wformat( L"%d" ) % fcn).str() );
		}
        if ( portfolio::Folium folium = folder.findFoliumByName( L"TIC/TIC.1" ) ) {
			if ( folium.empty() )
				processor->fetch( folium );
			processor->setCurrentSelection( folium );
		}
    }

    // show first spectrum on tree by default
    portfolio::Folder spectra = portfolio.findFolder( L"Spectra" );
    if ( !spectra.nil() ) {
        portfolio::Folio folio = spectra.folio();
        if ( !folio.empty() ) {
            processor->fetch( folio[0] );
			processor->setCurrentSelection( folio[0] );
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
MSProcessingWnd::handleSelectionChanged( Dataprocessor* /* processor */, portfolio::Folium& folium )
{
    drawIdx1_ = 0;
    drawIdx2_ = 0;

    if ( portfolio::Folder folder = folium.getParentFolder() ) {

        if ( folder.name() == L"Spectra" ) { // || folder.name() == L"Chromatograms" ) {

            if ( portfolio::is_type< adcontrols::MassSpectrumPtr >( folium ) ) {

                pProcessedSpectrum_ = std::make_pair( std::wstring(), std::shared_ptr< adcontrols::MassSpectrum >( 0 ) );
                pProfileSpectrum_ = std::make_pair( std::wstring(), std::shared_ptr< adcontrols::MassSpectrum >( 0 ) );
                pkinfo_ = std::make_pair( std::wstring(), std::shared_ptr< adcontrols::MSPeakInfo >( 0 ) );
                targeting_ = std::make_pair( std::wstring(), std::shared_ptr< adcontrols::Targeting >( 0 ) );

                if ( auto ptr = portfolio::get< adcontrols::MassSpectrumPtr >( folium ) ) {

                    idActiveFolium_ = folium.id();
                    idSpectrumFolium_ = folium.id();

                    draw_profile( folium.id(), ptr );

                    if ( auto fcentroid = portfolio::find_first_of( folium.attachments(), []( const portfolio::Folium& a ){
                                return a.name() == Constants::F_CENTROID_SPECTRUM; }) ) {
                        
                        if ( auto centroid = portfolio::get< adcontrols::MassSpectrumPtr >( fcentroid ) ) {
                            if ( centroid->isCentroid() ) {
                                draw2( centroid );
                                pProcessedSpectrum_ = std::make_pair( fcentroid.id(), centroid );
                            }
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
                            if ( auto fmth = portfolio::find_first_of( ftgt.attachments(), [] ( portfolio::Folium& a ){ return portfolio::is_type< adcontrols::ProcessMethodPtr >( a ); } ) )
                                if ( auto mth = portfolio::get< adcontrols::ProcessMethodPtr >( fmth ) )
                                    MainWindow::instance()->setProcessMethod( *mth );
                        }

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
            if ( portfolio::is_type< adcontrols::ChromatogramPtr >( folium ) ) {
                if ( auto ptr = portfolio::get< adcontrols::ChromatogramPtr > ( folium ) ) {
                    auto idStr = folium.attribute( L"protoId" );
                    int protoId = 0;
                    if ( !idStr.empty() )
                        protoId = std::stoi( idStr );
                    draw( ptr, protoId );
                    idActiveFolium_ = folium.id();
                    idChromatogramFolium( folder.id() );
                    if ( auto f = portfolio::find_first_of( folium.attachments(), []( portfolio::Folium& a ){
                                return portfolio::is_type< adcontrols::PeakResultPtr >( a ); }) ) {
                        auto pkresults = portfolio::get< adcontrols::PeakResultPtr >( f );
                        draw( pkresults );
                    }
                }
            }
        }
    }
}

void
MSProcessingWnd::handleAxisChanged( int axis )
{
    using adwplot::SpectrumWidget;

    axis_ = axis;

    pImpl_->set_time_axis( axis == AxisMZ ? false : true );
    pImpl_->profileSpectrum_->setAxis( axis == AxisMZ ? SpectrumWidget::HorizontalAxisMass : SpectrumWidget::HorizontalAxisTime, true );
    pImpl_->processedSpectrum_->setAxis( axis == AxisMZ ? SpectrumWidget::HorizontalAxisMass : SpectrumWidget::HorizontalAxisTime, true );
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
    pImpl_->focusedFcn( fcn );

    if ( auto pkinfo = pkinfo_.second.lock() ) {
        adcontrols::segment_wrapper< const adcontrols::MSPeakInfo > fpks( *pkinfo );
        auto pk = fpks[ fcn ].begin() + idx;
        pImpl_->currentChanged( *pk );
    }
    else if ( auto ms = pProcessedSpectrum_.second.lock() ) {
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
MSProcessingWnd::handleLockMass( const QVector< QPair<int, int> >& refs )
{
    if ( auto ms = pProcessedSpectrum_.second.lock() ) {

        adcontrols::lockmass lockmass;
        
        for ( auto ref: refs )
            adcontrols::lockmass::findReferences( lockmass, *ms, ref.first, ref.second );

        if ( lockmass.fit() ) {
            if ( lockmass( *ms ) ) {
                pImpl_->processedSpectrum_->setZoomBase( ms->getAcquisitionMassRange() );
                pImpl_->processedSpectrum_->update_annotation();

                if ( Dataprocessor * dp = SessionManager::instance()->getActiveDataprocessor() )
					dp->lockMassHandled( idSpectrumFolium_, ms, lockmass ); // update profile

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
MSProcessingWnd::handleCheckStateChanged( Dataprocessor* processor, portfolio::Folium& folium, bool isChecked )
{
    (void)processor;    (void)isChecked;

    portfolio::Folder folder = folium.getParentFolder();
	if ( !folder )
		return;
    if ( folder.name() == L"Chromatograms" ) {
        auto folio = folder.folio();
        int idx = 0;
        for ( auto& folium: folio ) {
            if ( folium.attribute( L"isChecked" ) == L"true" ) {
                if ( folium.empty() )
                    processor->fetch( folium );
                auto cptr = portfolio::get< adcontrols::ChromatogramPtr >( folium );
                pImpl_->ticPlot_->setData( cptr, idx );
            }
            ++idx;
        }
    }
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
MSProcessingWnd::selectedOnProfile( const QPointF& )
{
}

void
MSProcessingWnd::selectedOnProfile( const QRectF& rect )
{
	double x0 = pImpl_->profileSpectrum_->transform( QwtPlot::xBottom, rect.left() );
	double x1 = pImpl_->profileSpectrum_->transform( QwtPlot::xBottom, rect.right() );

	if ( int( std::abs( x1 - x0 ) ) > 2 ) {
        // todo: chromatogram creation by m/z|time range
        using namespace adcontrols::metric;

		QMenu menu;
        std::array< QAction *, 3 > fixedActions;
		fixedActions[ 0 ] = menu.addAction( QString::fromStdString( (boost::format("RMS in range %.3lf -- %.3lf(us)") % rect.left() % rect.right() ).str() ) );
		fixedActions[ 1 ] = menu.addAction( QString::fromStdString( (boost::format("Max value in range %.3lf -- %.3lf(us)") % rect.left() % rect.right() ).str() ) );
        fixedActions[ 2 ] = menu.addAction( "Frequency analysis" );

        std::pair<size_t, size_t> range;
        if ( auto ms = pProfileSpectrum_.second.lock() ) {
            if ( pImpl_->is_time_axis_ )
                range = std::make_pair( ms->getIndexFromTime( scale_to_base( rect.left(), micro ) ), ms->getIndexFromTime( scale_to_base( rect.right(), micro ) ) );
            else {
                const double * masses = ms->getMassArray();
                range = std::make_pair( std::distance( masses, std::lower_bound( masses, masses + ms->size(), rect.left() ) )
                                        , std::distance( masses, std::lower_bound( masses, masses + ms->size(), rect.right() ) ) );
            }

            QAction * selectedItem = menu.exec( QCursor::pos() );
            if ( fixedActions[ 0 ] == selectedItem ) {
                if ( compute_rms( scale_to_base( rect.left(), micro), scale_to_base( rect.right(), micro ) ) > 0 )
                    draw1();
                return;
            } else if ( fixedActions[ 1 ] == selectedItem ) {
                compute_minmax( scale_to_base( rect.left(), micro), scale_to_base( rect.right(), micro ) );
                draw1();
                return;
            } else if ( fixedActions[ 2 ] == selectedItem ) {
                std::vector< double > freq, power;
                double y_dc(0), y_nyquist(0);
                if ( power_spectrum( *ms, freq, power, range, y_dc, y_nyquist ) ) {
                    std::ostringstream o;
                    o << boost::format( "N=%d Power: DC=%.7g Nyquist=%.7g" ) % (freq.size() * 2) % y_dc % y_nyquist;
                    QString title = QString("[%1]&nbsp;&nbsp;&nbsp;&nbsp;%2").arg( MainWindow::makeDisplayName( idSpectrumFolium_ ), QString::fromStdString( o.str() ) );

                    pImpl_->pwplot_->setData( freq.size() - 1, freq.data() + 1, power.data() + 1 );
                    pImpl_->pwplot_->setTitle( title );

                    pImpl_->pwplot_->show();
                }
            }
        }

	} else {
		std::vector< std::wstring > models = adcontrols::MassSpectrometer::get_model_names();
        using adportable::utf;

        if ( ! models.empty() ) {
            QMenu menu;

            std::array< QAction *, 4 > fixedActions;
            fixedActions[ 0 ] = menu.addAction( "Correct baseline" );
            fixedActions[ 1 ] = menu.addAction( "Copy to Clipboard" );
            fixedActions[ 2 ] = menu.addAction( "Frequency analysis" );
            fixedActions[ 3 ] = menu.addAction( "Save SVG File..." );

            std::vector< QAction * > actions;
            for ( auto model: models ) {
                actions.push_back( menu.addAction( 
                                       ( boost::format( "Re-assign masses using %1% scan law" ) % utf::to_utf8( model ) ).str().c_str() ) );
            }

            QAction * selectedItem = menu.exec( QCursor::pos() );
            if ( fixedActions[ 0 ] == selectedItem ) {
                correct_baseline();
                draw1();
                return;
            } else if ( fixedActions[ 1 ] == selectedItem ) {
                adwplot::Dataplot::copyToClipboard( pImpl_->profileSpectrum_ );
                return;
            } else if ( fixedActions[ 2 ] == selectedItem ) {
                if ( auto ms = pProfileSpectrum_.second.lock() ) {
                    auto range = std::make_pair( size_t(0), ms->size() - 1 );
                    std::vector< double > freq, power;
                    double y_dc(0), y_nyquist(0);
                    if ( power_spectrum( *ms, freq, power, range, y_dc, y_nyquist ) ) {

                        std::ostringstream o;
                        o << boost::format( "N=%d Power: DC=%.7g Nyquist=%.7g" ) % (freq.size() * 2) % y_dc % y_nyquist;
                        QString title = QString("[%1]&nbsp;&nbsp;&nbsp;&nbsp;%2").arg( MainWindow::makeDisplayName( idSpectrumFolium_ ), QString::fromStdString( o.str() ) );

                        pImpl_->pwplot_->setData( freq.size() - 1, freq.data() + 1, power.data() + 1 );
                        pImpl_->pwplot_->setTitle( title );
                    }
                }
                return;
            } else if ( fixedActions[ 3 ] == selectedItem ) {
                QString name = QFileDialog::getSaveFileName( MainWindow::instance(), "Save SVG File", MainWindow::makePrintFilename( idSpectrumFolium_, L"_profile_" ), tr("SVG (*.svg)") );
                if ( ! name.isEmpty() )
					adwplot::Dataplot::copyImageToFile( pImpl_->profileSpectrum_, name, "svg" );
            }

            auto it = std::find_if( actions.begin(), actions.end(), [selectedItem]( const QAction *item ){
                    return item == selectedItem; }
                );

            if ( it != actions.end() ) {
                const std::wstring& model_name = models[ std::distance( actions.begin(), it ) ];
                assign_masses_to_profile( model_name );
                pImpl_->profileSpectrum_->replot();
            }
        }
    }
}

void
MSProcessingWnd::selectedOnPowerPlot( const QRectF& rect )
{
    QMenu menu;
    
    std::array< QAction *, 3 > fixedActions;
    fixedActions[ 0 ] = menu.addAction( "Copy to Clipboard" );
    fixedActions[ 1 ] = menu.addAction( "Save as SVG File..." );
    fixedActions[ 2 ] = menu.addAction( "Dismiss" );
    
    QAction * selectedItem = menu.exec( QCursor::pos() );
    if ( fixedActions[ 0 ] == selectedItem )
        adwplot::Dataplot::copyToClipboard( pImpl_->pwplot_ );
    else if ( fixedActions[ 1 ] == selectedItem ) {
        QString name = QFileDialog::getSaveFileName( MainWindow::instance(), "Save SVG File", MainWindow::makePrintFilename( idSpectrumFolium_, L"_power_" ), tr( "SVG (*.svg)" ) );
        if ( !name.isEmpty() )
            adwplot::Dataplot::copyImageToFile( pImpl_->pwplot_, name, "svg" );
    }
    else if ( fixedActions[ 2 ] == selectedItem )
        pImpl_->pwplot_->hide();
}

void
MSProcessingWnd::selectedOnProcessed( const QPointF& pos )
{
    ADTRACE() << "MSProcessingWnd::selectedOnProcessed: " << pos.x() << ", " << pos.y();
}

void
MSProcessingWnd::selectedOnProcessed( const QRectF& rect )
{
	double x0 = pImpl_->profileSpectrum_->transform( QwtPlot::xBottom, rect.left() );
	double x1 = pImpl_->profileSpectrum_->transform( QwtPlot::xBottom, rect.right() );

	if ( int( std::abs( x1 - x0 ) ) > 2 ) {
        // todo: chromatogram creation from base peak in range

        if ( auto ptr = pProcessedSpectrum_.second.lock() ) {
#if defined BASEPEAK_SELECTION
            // find base peak
			auto idx = adcontrols::segments_helper::base_peak_index( *ptr, rect.left(), rect.right() );
            double mass = adcontrols::segments_helper::get_mass( *ptr, idx );
			std::vector< std::tuple< int, double, double > > ranges( 1, std::make_tuple( idx.second, mass, 0.05 ) );
			Dataprocessor * processor = SessionManager::instance()->getActiveDataprocessor();
			DataprocessWorker::instance()->createChromatograms( processor, ranges );
#else
            std::vector< std::pair< int, int > > indecies;
            if ( adcontrols::segments_helper::selected_indecies( indecies, *ptr, rect.left(), rect.right(), rect.top() ) ) {
                std::vector< std::tuple< int, double, double > > ranges;
                for ( auto& index: indecies ) {
                    double mass = adcontrols::segments_helper::get_mass( *ptr, index );
                    ranges.push_back( std::make_tuple( index.second, mass, 0.005 ) );
                }
                Dataprocessor * processor = SessionManager::instance()->getActiveDataprocessor();
                DataprocessWorker::instance()->createChromatograms( processor, ranges );
            }
#endif
        }

    } else {
        QMenu menu;

        std::vector< QAction * > actions;
        actions.push_back( menu.addAction( "Copy to clipboard" ) );
        actions.push_back( menu.addAction( "Save as SVG File..." ) );
        actions.push_back( menu.addAction( "Create chromatograms" ) );

        QAction * selectedItem = menu.exec( QCursor::pos() );
        auto it = std::find_if( actions.begin(), actions.end(), [selectedItem]( const QAction *item ){
                return item == selectedItem; }
            );
        if ( it != actions.end() ) {

            if ( *it == actions[ 0 ] ) {

                adwplot::Dataplot::copyToClipboard( pImpl_->processedSpectrum_ );

            } else if ( *it == actions[ 1 ] ) {

                QString name = QFileDialog::getSaveFileName( MainWindow::instance(), "Save SVG File"
                                                             , MainWindow::makePrintFilename( idSpectrumFolium_, L"_processed_" ), tr("SVG (*.svg)") );
                if ( ! name.isEmpty() )
                    adwplot::Dataplot::copyImageToFile( pImpl_->profileSpectrum_, name, "svg" );

            } else if ( *it == actions[ 2 ] ) {

                QRectF rc = pImpl_->processedSpectrum_->zoomRect();

                if ( adcontrols::MassSpectrumPtr ptr = pProcessedSpectrum_.second.lock() ) {
					// create chromatograms for all peaks in current zoomed scope
					Dataprocessor * processor = SessionManager::instance()->getActiveDataprocessor();
					DataprocessWorker::instance()->createChromatograms( processor, ptr, rc.left(), rc.right() );
				}
            }
        }
    }
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

	QString formattedMethod;

    portfolio::Folio attachments = folium.attachments();
    portfolio::Folio::iterator it
        = portfolio::Folium::find<adcontrols::MassSpectrumPtr>( attachments.begin(), attachments.end() );
    if ( it != attachments.end() ) {
        adutils::MassSpectrumPtr ms = boost::any_cast< adutils::MassSpectrumPtr >( *it );
        const adcontrols::Descriptions& desc = ms->getDescriptions();
        for ( size_t i = 0; i < desc.size(); ++i ) {
            const adcontrols::Description& d = desc[i];
            if ( ! d.xml().empty() ) {
                formattedMethod.append( d.xml().c_str() ); // boost::serialization does not close xml correctly, so xmlFormatter raise an exception.
            }
        }
    }
    drawRect.setTop( drawRect.bottom() + 0.5 * resolution );
    drawRect.setHeight( printer.height() - drawRect.top() );
    QFont font = painter.font();
    font.setPointSize( 8 );
    painter.setFont( font );
    painter.drawText( drawRect, Qt::TextWordWrap, formattedMethod, &boundingRect );
}

bool
MSProcessingWnd::assign_masses_to_profile( const std::wstring& model_name )
{
    try {
        const adcontrols::MassSpectrometer& model = adcontrols::MassSpectrometer::get( model_name.c_str() );
        const adcontrols::ScanLaw& law = model.getScanLaw();
        ADTRACE() << model_name;

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
        }
	
        return true;
    } catch ( boost::exception& ex ) {
        ADERROR() << boost::diagnostic_information( ex );
        return assign_masses_to_profile();
    }
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
		prop.acceleratorVoltage( law.kAcceleratorVoltage() );
		prop.tDelay( law.tDelay() );
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
		x->addDescription( adcontrols::Description( L"process", o.str() ) );
	}
	return tic;
}

double
MSProcessingWnd::compute_rms( double s, double e )
{
	if ( auto ptr = this->pProfileSpectrum_.second.lock() ) {

		adcontrols::segment_wrapper< adcontrols::MassSpectrum > segments( *ptr );
        
        for ( auto& ms: segments ) {

            std::pair< size_t, size_t > range;
            if ( pImpl_->is_time_axis_ ) {
                range.first = ms.getIndexFromTime( s, false );
                range.second = ms.getIndexFromTime( e, true );
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
                
                ptr->addDescription( adcontrols::Description( L"process"
                                                              , (boost::wformat(L"RMS[%.3lf-%.3lf,N=%d]=%.3lf")
                                                                 % scale_to_micro(s) % scale_to_micro(e) % n % rms).str() ) );

                QString text = QString::fromStdString( ( boost::format("rms(start,end,N,rms)\t%.14f\t%.14f\t%d\t%.7f") % scale_to_micro(s) % scale_to_micro(e) % n % rms).str() );
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
                range.first = ms.getIndexFromTime( s, false );
                range.second = ms.getIndexFromTime( e, true );
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

                size_t index = std::distance( data.begin(), pair.second );
				double t = adcontrols::MSProperty::toSeconds( index, ms.getMSProperty().getSamplingInfo() );

                ptr->addDescription( adcontrols::Description( L"process"
					, (boost::wformat(L"MAX at %.4us=%.3f") % scale_to_micro(t) % result.second ).str() ) );
                
				QString text = QString::fromStdString( ( boost::format("max @\t%d\t%.14lf\t%.7f") % index % scale_to_micro(t) % result.second ).str() );
                QApplication::clipboard()->setText( text );

				return result;
            }
        }

    }
	return std::make_pair(0,0);
}

bool
MSProcessingWnd::power_spectrum( const adcontrols::MassSpectrum& ms
                                 , std::vector< double >& x, std::vector< double >& y
                                 , const std::pair< size_t, size_t >& range
                                 , double& dc, double& nyquist )
{
    const size_t size = range.second - range.first + 1;
    if ( size < 8 )
        return false;
    size_t n = 1;
    while ( size >> n )
        ++n;
    size_t N = 1 << ( n - 1 );

    std::vector< std::complex< double > > spc(N), fft(N);
	const double * intens = ms.getIntensityArray() + range.first;
	for ( size_t i = 0; i < N && i < size; ++i )
        spc[ i ] = std::complex< double >( intens[ i ] );
    adportable::fft::fourier_transform( fft, spc, false );

	const double T = N * ms.getMSProperty().getSamplingInfo().fSampInterval();
    x.resize( N / 2 );
    y.resize( N / 2 );
    for ( size_t i = 0; i < N / 2; ++i ) {
        y[i] = ( ( fft[ i ].real() * fft[ i ].real() ) + ( fft[ i ].imag() * fft[ i ].imag() ) ) / ( double(N) * N );
        x[i] = double( i ) / T * 1e-6; // MHz
    }
    dc = fft[0].real();
    nyquist = fft[ N / 2 ].real();

	return true;
}
