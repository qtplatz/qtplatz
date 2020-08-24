/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#include "processingwnd.hpp"
#include "document.hpp"
#include "malpixconstants.hpp"
#include <mpxwidgets/spectrogramplot.hpp>
#include <mpxcontrols/dataframe.hpp>
#include <mpxcontrols/population_protocol.hpp>
#include <mpxprocessor/processor.hpp>
#include <adcontrols/mappedspectrum.hpp>
#include <adcontrols/mappedspectra.hpp>
#include <adcontrols/mappedimage.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adinterface/signalobserver.hpp>
#include <adplot/chromatogramwidget.hpp>
#include <adplot/picker.hpp>
#include <adplot/plotcurve.hpp>
#include <adplot/spanmarker.hpp>
#include <adplot/spectrumwidget.hpp>
#include <adplot/zoomer.hpp>
#include <adportable/bzip2.hpp>
#include <adportable/debug.hpp>
#include <adportable/float.hpp>
#include <adportfolio/portfolio.hpp>
#include <adportfolio/folder.hpp>
#include <adportfolio/folium.hpp>
#include <adwidgets/progresswnd.hpp>
#include <qtwrapper/font.hpp>
#include <qtwrapper/progresshandler.hpp>
#include <coreplugin/minisplitter.h>
#include <coreplugin/progressmanager/progressmanager.h>
#include <QBoxLayout>
#include <QCoreApplication>
#include <QEvent>
#include <QKeyEvent>
#include <QSignalBlocker>
#include <boost/any.hpp>
#include <boost/exception/all.hpp>
#include <boost/format.hpp>
#include <condition_variable>
#include <iostream>
#include <numeric>
#include <mutex>
#include <thread>

namespace malpixproc {

    class ProcessingWnd::impl {
        ProcessingWnd * parent_;
    public:
        impl( ProcessingWnd * p ) : parent_( p )
            {}
        std::unique_ptr< adplot::ChromatogramWidget > timed_plot_;
        std::unique_ptr< adplot::SpectrumWidget > spectrum_plot_;
		std::unique_ptr< mpxwidgets::SpectrogramPlot > map_;

        std::weak_ptr< const adcontrols::MappedSpectra > matrix_;   // --> origin is the mpxprocessor::processor class
        std::weak_ptr< const adcontrols::MassSpectrum > ms_;        // --> origin is the mpxprocessor::processor class
        std::weak_ptr< const adcontrols::Chromatogram > chro_;      // --> origin is the mpxprocessor::processor class
        std::weak_ptr< const adcontrols::MassSpectrum > msOverlay_; // --> origin is in folum

        std::shared_ptr< const adcontrols::MassSpectrum > ms1_;     // histogram from malpix
        std::shared_ptr< const adcontrols::MassSpectrum > ms2_;     // cell selected histogram from malpix

        std::unique_ptr< adplot::SpanMarker > timedPlotMarker_;
        std::unique_ptr< adplot::SpanMarker > tofRangeMarker_;

        //bool cellSelectionEnabled_;
        QRect cellSelected_;
        std::pair< double, double > trigRange_; // time in seconds
        std::pair< double, double > tofRange_;
        static QwtText make_map_title( const std::pair< double, double >& trig, const std::pair< double, double >& tof );
        static QwtText make_spectrum_title( const std::pair< double, double >& trig );
    };
}

using namespace malpixproc;

ProcessingWnd::~ProcessingWnd()
{
    delete impl_;
}

ProcessingWnd::ProcessingWnd( QWidget *parent ) : QWidget( parent )
                                                , impl_( new impl( this ) )
{
    setContextMenuPolicy( Qt::CustomContextMenu );
    
    if ( auto splitter = new Core::MiniSplitter ) {
        
        if ( auto splitter2 = new Core::MiniSplitter ) {
            
            if ( (impl_->timed_plot_ = std::make_unique<adplot::ChromatogramWidget >( this )) ) {
                impl_->timed_plot_->setMinimumHeight( 80 );

                connect( impl_->timed_plot_.get(), SIGNAL( onSelected( const QRectF& ) )
                         , this, SLOT( handleSelectedOnChromatogram( const QRectF& ) ) );

                connect( this, &ProcessingWnd::nextMappedSpectra, this, &ProcessingWnd::handleNextMappedSpectra );
                connect( this, &ProcessingWnd::tofMoved, this, &ProcessingWnd::handleTofMoved );
            }
            
            if ( (impl_->spectrum_plot_ = std::make_unique< adplot::SpectrumWidget >( this )) ) {
                
                impl_->spectrum_plot_->setMinimumHeight( 80 );
                impl_->spectrum_plot_->setAxis( adplot::SpectrumWidget::HorizontalAxisTime );
                impl_->spectrum_plot_->setZoomBase( std::make_pair( 0, 100 ) );
                impl_->spectrum_plot_->enableAxis( QwtPlot::yRight );
                impl_->spectrum_plot_->setAxisTitle( QwtPlot::xBottom, QwtText( "Time-of-flight(&mu;s)", QwtText::RichText ) );

                connect( impl_->spectrum_plot_.get(), SIGNAL( onSelected( const QRectF& ) )
                         , this, SLOT( handleSelectedOnSpectrum( const QRectF& ) ) );
            }
            
            splitter2->addWidget( impl_->timed_plot_.get() );
            splitter2->addWidget( impl_->spectrum_plot_.get() );
            splitter2->setOrientation( Qt::Vertical );

            splitter->addWidget( splitter2 );
        }

        impl_->map_ = std::make_unique< mpxwidgets::SpectrogramPlot >( this );
        impl_->map_->installEventFilter( this );
        impl_->timed_plot_->installEventFilter( this );
        impl_->spectrum_plot_->installEventFilter( this );

        connect( impl_->map_.get(), &mpxwidgets::SpectrogramPlot::cellSelected, this, &ProcessingWnd::handleCellSelected );
        connect( this, &ProcessingWnd::cellMoved, this, &ProcessingWnd::handleCellMoved );
        
        splitter->addWidget( impl_->map_.get() );
        splitter->setOrientation( Qt::Horizontal );

        auto layout = new QVBoxLayout( this );
        layout->setMargin( 0 );
        layout->setSpacing( 0 );
        layout->addWidget( splitter );

        //
        if ( ( impl_->timedPlotMarker_ =
               std::make_unique< adplot::SpanMarker >( QColor( 0xff, 0xa5, 0x00, 0x80 ), QwtPlotMarker::VLine, 2.5 ) ) ) { // orange
            impl_->timedPlotMarker_->setValue( 0.0, 0.0 );
            impl_->timedPlotMarker_->attach( impl_->timed_plot_.get() );
        }
        
        if ( ( impl_->tofRangeMarker_ =
               std::make_unique< adplot::SpanMarker >( QColor( 0x7c, 0xfc, 0x00, 0x80 ), QwtPlotMarker::VLine, 2.0 ) ) ) { // lawngreen
            impl_->tofRangeMarker_->setValue( 0.0, 0.0 );
            impl_->tofRangeMarker_->attach( impl_->spectrum_plot_.get() );
        }
        
    }
}


bool
ProcessingWnd::eventFilter( QObject * object, QEvent * event )
{
    if ( event->type() == QEvent::KeyPress ) {

        if ( object == impl_->timed_plot_.get() ) {

            if ( QKeyEvent * keyEvent = static_cast<QKeyEvent *>( event ) ) {
                switch ( keyEvent->key() ) {
                case Qt::Key_Left:  emit nextMappedSpectra( false ); break;
                case Qt::Key_Right: emit nextMappedSpectra( true ); break;
                }                
            }
        } else if ( object == impl_->spectrum_plot_.get() ) {

            if ( QKeyEvent * keyEvent = static_cast<QKeyEvent *>( event ) ) {
                switch ( keyEvent->key() ) {
                case Qt::Key_Left:  emit tofMoved( -16 ); break;
                case Qt::Key_Right: emit tofMoved( 16 ); break;
                }                
            }

        } else if ( object == impl_->map_.get() ) {

            if ( QKeyEvent * keyEvent = static_cast<QKeyEvent *>( event ) ) {
                switch ( keyEvent->key() ) {
                case Qt::Key_Left:  emit cellMoved(-1,  0 ); break;
                case Qt::Key_Right: emit cellMoved( 1,  0 ); break;
                case Qt::Key_Up:    emit cellMoved( 0, -1 ); break;
                case Qt::Key_Down:  emit cellMoved( 0,  1 ); break;                    
                }
            }
        }
    }
    return QWidget::eventFilter( object, event );
}

void
ProcessingWnd::handleProcessorChanged()
{
    handleSelectedOnChromatogram( QRectF() );
}

void
ProcessingWnd::handleCheckStateChanged( const portfolio::Folium& folium )
{
    if ( folium.parentFolder().name() == L"Spectra" ) {
        if ( folium.attribute( L"isChecked" ) == L"true" ) {
            if ( portfolio::is_type< std::shared_ptr< const adcontrols::MassSpectrum > >( folium ) ) {
                if ( auto ms = portfolio::get< std::shared_ptr< const adcontrols::MassSpectrum > >( folium ) ) {
                    impl_->msOverlay_ = ms;
                }
            } else if ( portfolio::is_type< std::shared_ptr< adcontrols::MassSpectrum > >( folium ) ) {
                if ( auto ms = portfolio::get< std::shared_ptr< adcontrols::MassSpectrum > >( folium ) ) {
                    impl_->msOverlay_ = ms;
                }
            }
        } else {
            impl_->msOverlay_.reset();
            impl_->spectrum_plot_->clear();
        }
    }
}

void
ProcessingWnd::handleDataChanged( const portfolio::Folium& folium )
{
    if ( folium.parentFolder().name() == L"Chromatograms" ) {

        if ( portfolio::is_type< std::shared_ptr< const adcontrols::Chromatogram > >( folium ) ) {
            if ( auto chro = portfolio::get< std::shared_ptr< const adcontrols::Chromatogram > >( folium ) ) {
                impl_->chro_ = chro;
                impl_->timed_plot_->setData( chro );
            }
        } else if ( portfolio::is_type< std::shared_ptr< adcontrols::Chromatogram > >( folium ) ) {
            if ( auto chro = portfolio::get< std::shared_ptr< adcontrols::Chromatogram > >( folium ) ) {
                impl_->chro_ = chro;
                impl_->timed_plot_->setData( chro );
            }
        }

    } else if ( folium.parentFolder().name() == L"Spectra" ) {

        if ( portfolio::is_type< std::shared_ptr< const adcontrols::MassSpectrum > >( folium ) ) {
            if ( auto ms = portfolio::get< std::shared_ptr< const adcontrols::MassSpectrum > >( folium ) ) {
                impl_->ms_ = ms;
                impl_->spectrum_plot_->setData( ms, 2, true );
            }
        } else if ( portfolio::is_type< std::shared_ptr< adcontrols::MassSpectrum > >( folium ) ) {
            if ( auto ms = portfolio::get< std::shared_ptr< adcontrols::MassSpectrum > >( folium ) ) {
                impl_->ms_ = ms;
                impl_->spectrum_plot_->setData( ms, 2, true );
            }
        }
        
        impl_->spectrum_plot_->setAlpha( 2, 0x20 );
    }
}

QwtText
ProcessingWnd::impl::make_map_title( const std::pair< double, double >& trig, const std::pair< double, double >& tof )
{
    using namespace adcontrols::metric;
    
    QString title;

    if ( adportable::compare<double>::approximatelyEqual( trig.first, trig.second ) ) {
        
        title = QString( "Trig. at %1(s)" ).arg( QString::number( trig.first, 'f', 3) );

    } else {

        title = QString( "Trig. range %1-%2(s)" ).arg( QString::number( trig.first, 'f', 3 ),
                                                                QString::number( trig.second, 'f', 3 ) );
    }

    if ( tof.first >= 0.0 ) {
        title += QString( "; TOF range %1-%2(&mu;s)" ).arg( QString::number( scale_to_micro( tof.first ), 'f', 2 )
                                                      , QString::number( scale_to_micro( tof.second ), 'f', 2 ) );
    }

    QwtText text( title, QwtText::RichText );
    text.setFont( qtwrapper::font()( QFont(), qtwrapper::fontSizeNormal, qtwrapper::fontPlotTitle ) );
    return text;
}

QwtText
ProcessingWnd::impl::make_spectrum_title( const std::pair< double, double >& trig )
{
    QString title;

    if ( adportable::compare<double>::approximatelyEqual( trig.first, trig.second ) ) {
        
        title = QString( "Trig. at %1(s)" ).arg( QString::number( trig.first, 'f', 3) );
        
    } else {
        
        title = QString( "Trig. range %1-%2(s)" ).arg( QString::number( trig.first, 'f', 3 ),
                                                           QString::number( trig.second, 'f', 3 ) );
    }
    return QwtText( title, QwtText::RichText );
}

void
ProcessingWnd::setData( std::shared_ptr< const adcontrols::MappedSpectra >&& matrix
                        , const std::pair< double, double >& trig
                        , const std::pair< double, double >& tof )
{
    QwtText title;
    
    impl_->matrix_ = matrix;

    emit document::instance()->setEnabled( 0, false ); // uncheck 'Histogram data range' := map rect
    emit document::instance()->setEnabled( 1, false ); // uncheck 'Tof range'

    // plot on map (contour)
    if ( auto image = std::make_shared< adcontrols::MappedImage >() ) {
        if ( image->merge( *matrix ) ) {
            document::instance()->setMappedImage( image, trig, tof );
            impl_->map_->setData( std::move( image ) );
            impl_->map_->setTitle( impl::make_map_title( trig, tof ) );
        }
    }

    // plot spectrum
    if ( auto sp = std::make_unique< adcontrols::MappedSpectrum >() ) {
        matrix->sum_in_range( *sp, 0, 0, matrix->size1(), matrix->size2() );
        if ( auto ms = std::make_shared< adcontrols::MassSpectrum >() ) {
            if ( sp->transform( *ms ) ) {

                impl_->ms1_ = ms;
                impl_->ms2_.reset(); // clear cell selected spectrum

                impl_->spectrum_plot_->setData( ms, 0 );
                impl_->spectrum_plot_->setTitle( impl::make_spectrum_title( trig ).text() );

                impl_->tofRangeMarker_->setValue( ms->getTime( 0 ), ms->getTime( ms->size() - 1 ) );
                impl_->spectrum_plot_->replot();
            }

        }
    }

    impl_->timedPlotMarker_->setValue( trig.first, trig.second );
    impl_->timed_plot_->replot();
}


// from adplot::ChromatogramWidget
void
ProcessingWnd::handleSelectedOnChromatogram( const QRectF& rc )
{
    if ( auto doc = document::instance()->currentProcessor() ) {

        auto range = std::make_pair( doc->findElapsedTime( rc.left() ), doc->findElapsedTime( rc.right() ) );

        if ( auto matrix = doc->mappedSpectra( range.first, range.second ) ) {

            const auto& sp = (*matrix)( 0, 0 );
         
            impl_->tofRange_ = sp.acqTimeRange(); // set full tof range 
            impl_->trigRange_ = std::make_pair( range.first->time_since_inject(), range.second->time_since_inject() );

            setData( std::move( matrix ), impl_->trigRange_, impl_->tofRange_ );

        }

    }
}

void
ProcessingWnd::handleNextMappedSpectra( bool forward )
{
    if ( auto doc = document::instance()->currentProcessor() ) {

        auto range = doc->selectedRangeOnTimedTrace();
        if ( forward ) {
            range.second = ++range.first;
        } else {
            range.first = --range.second;
        }

        if ( auto matrix = doc->mappedSpectra( range.first, range.second ) ) {

            QRectF base = impl_->timed_plot_->zoomer()->zoomBase();
            QRectF rc = impl_->timed_plot_->zoomRect();

            if ( base.width() / 2 > rc.width() ) {

                double left = range.first->time_since_inject() - rc.width() / 2;
                impl_->timed_plot_->zoomer()->moveTo( QPointF( left, 0.0 ) );
                
            }

            impl_->trigRange_ = { range.first->time_since_inject(), range.second->time_since_inject() };

            setData( std::move( matrix ), impl_->trigRange_, impl_->tofRange_ );

        }
    }
}

void
ProcessingWnd::handleTofMoved( int step )
{
    using namespace adcontrols::metric;

    if ( auto matrix = impl_->matrix_.lock() ) {

        const auto& sp = (*matrix)( 0, 0 );
        double distance = sp.samplingInterval() * step;
        auto acqRaneg = sp.acqTimeRange();

        std::pair< double, double > tof = { impl_->tofRange_.first + distance, impl_->tofRange_.second + distance };

        if ( ( step > 0 ) && ( tof.second > acqRaneg.second ) ) // over range
            return;
        else if ( tof.first < acqRaneg.first ) // under range
            return;

        impl_->tofRange_ = tof;

        double ctof  = tof.first + ( tof.second - tof.first ) / 2;
        double width = tof.second - tof.first;

        if ( auto image = std::make_shared< adcontrols::MappedImage >() ) {        

            if ( image->merge( *matrix, ctof, width ) ) {
                impl_->map_->setData( std::move( image ) );
                impl_->map_->setTitle( impl::make_map_title( impl_->trigRange_, tof ) );

                QRectF rc = impl_->spectrum_plot_->zoomRect();
                QRectF tofRect( scale_to_micro( tof.first ), rc.y(), scale_to_micro( tof.second - tof.first ), rc.height() );

                if ( rc.right() < tofRect.right() ) {
                    impl_->spectrum_plot_->zoomer()->moveBy( tofRect.right() - rc.right(), 0 );
                } else if ( rc.left() > tofRect.left() ) {
                    impl_->spectrum_plot_->zoomer()->moveBy( tofRect.left() - rc.left(), 0 );
                }

                impl_->tofRangeMarker_->setValue( scale_to_micro( tof.first ), scale_to_micro( tof.second ) );
                impl_->spectrum_plot_->replot();
            }
        }
    }
}


void
ProcessingWnd::handleSelectedOnSpectrum( const QRectF& rc )
{
    // tof range seleected --> update image map with given tof range
    if ( auto matrix = impl_->matrix_.lock() ) {

        if ( auto image = std::make_shared< adcontrols::MappedImage >() ) {

            emit document::instance()->selRangeOnSpectrum( rc );
            emit document::instance()->setEnabled( 1, true ); // check (enable) tof range for map

            using namespace adcontrols::metric;

            double tof = scale_to_base( rc.left() + rc.width() / 2, micro );
            double width = scale_to_base( rc.width(), micro );
            
            if ( image->merge( *matrix, tof, width ) ) {
                using namespace adcontrols::metric;

                impl_->tofRange_ = { scale_to_base( rc.left(), micro ), scale_to_base( rc.right(), micro ) };

                document::instance()->setMappedImage( image, impl_->trigRange_, impl_->tofRange_ );
                
                impl_->map_->setData( std::move( image ) );
                impl_->map_->setTitle( impl::make_map_title( impl_->trigRange_, impl_->tofRange_ ) );

                impl_->tofRangeMarker_->setValue( rc.left(), rc.right() );
                impl_->spectrum_plot_->replot();
            }

        }
    }    
}

void
ProcessingWnd::handleAxisChanged()
{
}

void
ProcessingWnd::handleCellSelected( const QRect& rc )
{
    impl_->cellSelected_ = rc;

    if ( auto matrix = impl_->matrix_.lock() ) {

        if ( auto sp = std::make_unique< adcontrols::MappedSpectrum >() ) {

            matrix->sum_in_range( *sp, rc.top(), rc.left(), rc.height(), rc.width() );

            if ( auto ms = std::make_shared< adcontrols::MassSpectrum >() ) {

                if ( sp->transform( *ms ) ) {

                    impl_->ms2_ = ms;
                    impl_->spectrum_plot_->clear(); // clear all (if overlay waveform exists)

                    impl_->spectrum_plot_->setData( impl_->ms1_, 0, false ); // full spectrum
                    impl_->spectrum_plot_->setAlpha( 0, 0x40 );
                    
                    impl_->spectrum_plot_->setData( impl_->ms2_, 1, true );   // cell selected spectrum

                    emit document::instance()->setEnabled( 0, true ); // check (enable) 'Histogram data range' := map rect
                }
            }

        }
        
    }
}

void
ProcessingWnd::handleCellMoved( int hor, int vert )
{
    QRect rc( impl_->cellSelected_ );

    rc.moveTo( rc.left() + hor, rc.top() + vert );
    QRect full( 0, 0, 64, 64 );

    if ( full.contains( rc ) ) {
        QSignalBlocker block( impl_->map_.get() );
        impl_->map_->setCellSelection( rc );
        handleCellSelected( rc );
    }
}

void
ProcessingWnd::setHistogramWindow( double tof, double window )
{
    QRectF rc( tof - window / 2, 0, window, 0 );
    handleSelectedOnSpectrum( rc );
}

void
ProcessingWnd::setEnabled( int id, bool enable ) // check/uncheck map rect (0) or tof range(1)
{
    if ( id == 0 ) {

        if ( auto matrix = impl_->matrix_.lock() ) {

            impl_->spectrum_plot_->clear();
            
            if ( auto sp1 = std::make_unique< adcontrols::MappedSpectrum >() ) {
                matrix->sum_in_range( *sp1, 0, 0, -1, -1 );
                if ( auto ms = std::make_shared< adcontrols::MassSpectrum >() ) {
                    if ( sp1->transform( *ms ) )
                        impl_->spectrum_plot_->setData( ms, 0 );
                }
                
                if ( enable ) {

                    impl_->spectrum_plot_->setAlpha( 0, 0x40 );
                    
                    const QRect& rc = impl_->cellSelected_;
                    if ( auto sp2 = std::make_unique< adcontrols::MappedSpectrum >() ) {
                        matrix->sum_in_range( *sp2, rc.top(), rc.left(), rc.height(), rc.width() );
                        if ( auto ms = std::make_shared< adcontrols::MassSpectrum >() ) {
                            if ( sp2->transform( *ms ) )
                                impl_->spectrum_plot_->setData( ms, 1, true );
                        }
                    }
                }
            }
        }
        
    } else {
        
        // tof range
        if ( auto matrix = impl_->matrix_.lock() ) {

            ADDEBUG() << "acq time range: " << matrix->acqTimeRange().first << ", " << matrix->acqTimeRange().second;
            ADDEBUG() << "tof time range: " << impl_->tofRange_.first << ", " << impl_->tofRange_.second;

            std::pair< double, double > range = enable ? ( impl_->tofRange_ ) : matrix->acqTimeRange();
            double width = range.second - range.first;
            
            if ( auto image = std::make_shared< adcontrols::MappedImage >() ) {
                if ( enable ? image->merge( *matrix, range.first + width / 2, width ) : image->merge( *matrix ) ) {

                    impl_->map_->setData( std::move( image ) );
                    impl_->map_->setTitle( impl::make_map_title( impl_->trigRange_, range ) );

                }
            }
        }
    }
}
