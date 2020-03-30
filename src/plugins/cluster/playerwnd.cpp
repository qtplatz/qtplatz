/**************************************************************************
** Copyright (C) 2010-2020 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2020 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "playerwnd.hpp"
#include "document.hpp"
#include "constants.hpp"
#include "player.hpp"
#include "uimediator.hpp"
#include <adcontrols/contoursmethod.hpp>
#include <adcontrols/massspectra.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adinterface/signalobserver.hpp>
#include <adplot/chromatogramwidget.hpp>
#include <adplot/picker.hpp>
#include <adplot/plotcurve.hpp>
#include <adplot/spanmarker.hpp>
#include <adplot/spectrogramdata.hpp>
#include <adplot/spectrogramwidget.hpp>
#include <adplot/spectrumwidget.hpp>
#include <adplot/zoomer.hpp>
#include <adportable/bzip2.hpp>
#include <adportable/debug.hpp>
#include <adportable/float.hpp>
#include <adportfolio/portfolio.hpp>
#include <adportfolio/folder.hpp>
#include <adportfolio/folium.hpp>
#include <adprocessor/dataprocessor.hpp>
#include <adcv/transform.hpp>
#include <adcv/imagewidget.hpp>
#include <adcv/imfilter.hpp>
#include <adwidgets/progresswnd.hpp>
#include <qtwrapper/font.hpp>
#include <qtwrapper/progresshandler.hpp>
#include <qwt_plot_renderer.h>
#include <coreplugin/minisplitter.h>
#include <coreplugin/progressmanager/progressmanager.h>
#include <QBoxLayout>
#include <QCoreApplication>
#include <QEvent>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QPrinter>
#include <QPainter>
#include <QPrinter>
#include <QSignalBlocker>
#include <boost/any.hpp>
#include <boost/exception/all.hpp>
#include <boost/format.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <condition_variable>
#include <iostream>
#include <numeric>
#include <mutex>
#include <thread>

#ifdef WIN32
namespace adcv {
    extern template class __declspec(dllimport) imfilter< QImage, imGrayScale >;
}
#endif

namespace cluster {

    class SpectrogramData;

    class PlayerWnd::impl {
        PlayerWnd * parent_;
    public:
        impl( PlayerWnd * p ) : parent_( p )
                              , avgCount_( 0 )
                              , filters_( 0 )
                              , zAutoScale_( true )
                              , zScale_( 1.0 )
            {}
        std::unique_ptr< adplot::ChromatogramWidget > timed_plot_;
        std::array< std::unique_ptr< adplot::SpectrumWidget >, 2 > splots_;
        std::unique_ptr< adcv::ImageWidget > img_;
		std::unique_ptr< adplot::SpectrogramWidget > map_;

        std::unique_ptr< SpectrogramData > sgData_;
        //std::weak_ptr< const adcontrols::MappedImage > image_;      // if Moments was checked
        //std::weak_ptr< const adcontrols::MappedSpectra > spectra_;  // --> origin is the mpxprocessor::processor class
        std::weak_ptr< const adcontrols::MassSpectrum > ms_;        // --> origin is the mpxprocessor::processor class
        std::weak_ptr< const adcontrols::Chromatogram > chro_;      // --> origin is the mpxprocessor::processor class
        std::weak_ptr< const adcontrols::MassSpectrum > msOverlay_; // --> origin is in folum

        std::shared_ptr< const adcontrols::MassSpectrum > ms1_;     // histogram from malpix
        std::shared_ptr< const adcontrols::MassSpectrum > ms2_;     // cell selected histogram from malpix

        std::unique_ptr< adplot::SpanMarker > timedPlotMarker_;
        std::unique_ptr< adplot::SpanMarker > tofRangeMarker_;
        size_t avgCount_;
        uint32_t filters_; // 1 = Gray, 2 = DFT, 4 = Blur
        bool zAutoScale_;
        double zScale_;

        //bool cellSelectionEnabled_;
        QRect cellSelected_;
        std::pair< double, double > trigRange_; // time in seconds
        std::pair< double, double > tofRange_;
        static QwtText make_map_title( const std::pair< double, double >& trig, const std::pair< double, double >& tof );
        static QwtText make_spectrum_title( const std::pair< double, double >& trig );
        void setImage( const boost::numeric::ublas::matrix< double >&, double maxScale );
        void replotImage();
    };

    class SpectrogramData : public adplot::SpectrogramData {
        SpectrogramData( const SpectrogramData& ) = delete;
    public:
        SpectrogramData( std::shared_ptr< const adcontrols::MassSpectra >&& spectra )
            : spectra_( spectra )
            , m_( 1024, spectra->size() )
            , xlimits_( spectra_->x_left(), spectra_->x_right() )
            , ylimits_( spectra_->lower_mass(), spectra_->upper_mass() )  {
            updateData();
        }

        double value( double x, double y ) const override {
			size_t ix = dx( x );
			size_t iy = dy( y );
            return m_( iy, ix );
        }
        QRectF boundingRect() const override {
            return QRectF( xlimits_.first, ylimits_.first, xlimits_.second - xlimits_.first, ylimits_.second - ylimits_.first );            
        }
        bool zoomed( const QRectF& rc ) override {
            xlimits_ = std::make_pair( rc.left(), rc.right() );
            ylimits_ = std::make_pair( rc.top(), rc.bottom() );
            updateData();
            return true;
        }
        const boost::numeric::ublas::matrix< double >& matrix() const { return  m_; }
    private:
        std::shared_ptr< const adcontrols::MassSpectra > spectra_;
        boost::numeric::ublas::matrix< double > m_;
        std::pair< double, double > xlimits_, ylimits_;
        size_t size1_;
        size_t size2_;
        
        size_t dx( double x ) const {
            size_t d = ((x - xlimits_.first) / ( xlimits_.second - xlimits_.first )) * ( size1_ - 1 );
			if ( d > m_.size1() - 1 )
				return m_.size1() - 1;
			return d;
        }
        size_t dy( double y ) const {
            size_t d = ((y - ylimits_.first) / ( ylimits_.second - ylimits_.first )) * ( m_.size2() - 1 );
			if ( d > m_.size2() - 1 )
				return m_.size2() - 1;
            return d;
        }
        void updateData() {
            m_.clear();

            size_t id1 = std::distance( spectra_->x().begin(), std::lower_bound( spectra_->x().begin(), spectra_->x().end(), xlimits_.first ) );
            size_t id2 = std::distance( spectra_->x().begin(), std::lower_bound( spectra_->x().begin(), spectra_->x().end(), xlimits_.second ) );
            size1_ = std::min( m_.size1(), id2 - id1 + 1 );

            double z_max = std::numeric_limits<double>::lowest();
            size_t id = 0;
            for ( auto& ms: *spectra_ ) {
                double x = spectra_->x()[ id++ ];

                if ( xlimits_.first <= x && x <= xlimits_.second ) {
                    size_t ix = dx(x);

                    adcontrols::segment_wrapper< const adcontrols::MassSpectrum > segs( *ms );
                    for ( auto& seg: segs ) {
                        for ( size_t i = 0; i < seg.size(); ++i ) {
                            double m = seg.mass( i );
                            if ( ylimits_.first < m && m < ylimits_.second ) {
                                size_t iy = dy(m);
                                m_( iy, ix ) += seg.intensity( i );
                                z_max = std::max( z_max, m_( iy, ix ) );
                            }
                        }
                    }
                }
            }
            setInterval( Qt::XAxis, QwtInterval( spectra_->x_left(), spectra_->x_right() ) );   // time (sec -> min)
            setInterval( Qt::YAxis, QwtInterval( spectra_->lower_mass(), spectra_->upper_mass() ) ); // m/z
            setInterval( Qt::ZAxis, QwtInterval( 0.0, z_max ) );
        }
    };
}

using namespace cluster;

PlayerWnd::~PlayerWnd()
{
    delete impl_;
}

PlayerWnd::PlayerWnd( QWidget *parent ) : QWidget( parent )
                                                , impl_( new impl( this ) )
{
    setContextMenuPolicy( Qt::CustomContextMenu );

    if ( auto splitter = new Core::MiniSplitter ) {

        if ( auto splitter2 = new Core::MiniSplitter ) {

            if ( (impl_->timed_plot_ = std::make_unique<adplot::ChromatogramWidget >( this )) ) {
                impl_->timed_plot_->setMinimumHeight( 80 );

                connect( impl_->timed_plot_.get(), SIGNAL( onSelected( const QRectF& ) )
                         , this, SLOT( handleSelectedOnChromatogram( const QRectF& ) ) );

                connect( this, &PlayerWnd::nextMappedSpectra, this, &PlayerWnd::handleNextMappedSpectra );
                connect( this, &PlayerWnd::tofMoved, this, &PlayerWnd::handleTofMoved );
            }

            for ( auto& plot: impl_->splots_ ) {
                if (( plot = std::make_unique< adplot::SpectrumWidget >( this ) )) {

                    plot->setMinimumHeight( 80 );
                    plot->setAxis( adplot::SpectrumWidget::HorizontalAxisTime );
                    plot->setZoomBase( std::make_pair( 0, 100 ) );
                    plot->enableAxis( QwtPlot::yRight );
                    plot->setAxisTitle( QwtPlot::xBottom, QwtText( "Time-of-flight(&mu;s)", QwtText::RichText ) );
                    plot->installEventFilter( this );

                    connect( plot.get(), SIGNAL( onSelected( const QRectF& ) )
                             , this, SLOT( handleSelectedOnSpectrum( const QRectF& ) ) );
                    splitter2->addWidget( plot.get() );
                }
            }
            splitter2->setOrientation( Qt::Vertical );
            splitter->addWidget( splitter2 );
        }

        connect( impl_->splots_[ 0 ].get(), &adplot::SpectrumWidget::onSelected
                 , this, [&]( const QRectF& rc ){ emit timeRangeSelectedOnSpectrum( 0, rc ); });

        impl_->img_ = std::make_unique< adcv::ImageWidget >( this );
        impl_->map_ = std::make_unique< adplot::SpectrogramWidget >( this );
        //impl_->map_->installEventFilter( this );
        //connect( impl_->map_.get(), SIGNAL( onSelected( const QPointF& ) ), this, SLOT( handleSelected( const QPointF& ) ) );
        //connect( impl_->map_.get(), SIGNAL( onSelected( const QRectF& ) ), this, SLOT( handleSelected( const QRectF& ) ) );
        

        impl_->timed_plot_->installEventFilter( this );

        //connect( impl_->map_.get(), &mpxwidgets::SpectrogramPlot::cellSelected, this, &PlayerWnd::handleCellSelected );
        connect( this, &PlayerWnd::cellMoved, this, &PlayerWnd::handleCellMoved );

        if ( auto splitter3 = new Core::MiniSplitter ) {
            splitter3->addWidget( impl_->timed_plot_.get() );

            if ( auto widget = new QWidget ) {
                if ( auto layout = new QHBoxLayout( widget ) ) {
                    layout->addWidget( impl_->img_.get() );
                    layout->addWidget( impl_->map_.get() );
                    splitter3->addWidget( widget );
                }
            }
            splitter3->setOrientation( Qt::Vertical );
            splitter3->setStretchFactor( 0, 0 );
            splitter3->setStretchFactor( 1, 2 );
            splitter->addWidget( splitter3 );
        }

        //splitter->addWidget( impl_->map_.get() );
        splitter->setOrientation( Qt::Horizontal );

        auto layout = new QVBoxLayout( this );
        layout->setMargin( 0 );
        layout->setSpacing( 0 );
        layout->addWidget( splitter );

        //
        if ( ( impl_->timedPlotMarker_ =
               std::make_unique< adplot::SpanMarker >( QColor( 0xff, 0xa5, 0x00, 0x80 ), QwtPlotMarker::VLine, 2.5 ) ) ) { // orange
            impl_->timedPlotMarker_->setXValue( 0.0, 0.0 );
            impl_->timedPlotMarker_->attach( impl_->timed_plot_.get() );
        }

        if ( ( impl_->tofRangeMarker_ =
               std::make_unique< adplot::SpanMarker >( QColor( 0x7c, 0xfc, 0x00, 0x80 ), QwtPlotMarker::VLine, 2.0 ) ) ) { // lawngreen
            impl_->tofRangeMarker_->setXValue( 0.0, 0.0 );
            impl_->tofRangeMarker_->attach( impl_->splots_[ 0 ].get() );
        }

    }
}


bool
PlayerWnd::eventFilter( QObject * object, QEvent * event )
{
    if ( event->type() == QEvent::KeyPress ) {

        if ( object == impl_->timed_plot_.get() ) {

            if ( QKeyEvent * keyEvent = static_cast<QKeyEvent *>( event ) ) {
                switch ( keyEvent->key() ) {
                case Qt::Key_Left:  emit nextMappedSpectra( false ); break;
                case Qt::Key_Right: emit nextMappedSpectra( true ); break;
                }
            }
        } else if ( object == impl_->splots_[ 0 ].get() ) {

            if ( QKeyEvent * keyEvent = static_cast<QKeyEvent *>( event ) ) {
                switch ( keyEvent->key() ) {
                case Qt::Key_Left:  emit tofMoved( -16 ); break;
                case Qt::Key_Right: emit tofMoved( 16 ); break;
                }
            }

        } //else if ( object == impl_->map_.get() ) {

        //     if ( QKeyEvent * keyEvent = static_cast<QKeyEvent *>( event ) ) {
        //         switch ( keyEvent->key() ) {
        //         case Qt::Key_Left:  emit cellMoved(-1,  0 ); break;
        //         case Qt::Key_Right: emit cellMoved( 1,  0 ); break;
        //         case Qt::Key_Up:    emit cellMoved( 0, -1 ); break;
        //         case Qt::Key_Down:  emit cellMoved( 0,  1 ); break;
        //         }
        //     }
        // }
    }
    return QWidget::eventFilter( object, event );
}

void
PlayerWnd::handleProcessorChanged()
{
    handleSelectedOnChromatogram( QRectF() );
}

void
PlayerWnd::handleCheckStateChanged( const portfolio::Folium& folium )
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
            for ( auto& splot: impl_->splots_ )
                splot->clear();
        }
    }
}

void
PlayerWnd::handleDataChanged( const portfolio::Folium& folium )
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
                impl_->splots_[ 1 ]->setData( ms, 0, false );
            }
        } else if ( portfolio::is_type< std::shared_ptr< adcontrols::MassSpectrum > >( folium ) ) {
            if ( auto ms = portfolio::get< std::shared_ptr< adcontrols::MassSpectrum > >( folium ) ) {
                impl_->ms_ = ms;
                impl_->splots_[ 1 ]->setData( ms, 0, false );
            }
        }
        // impl_->splots_[ 1 ]->setAlpha( 2, 0x20 );
    } else if ( ( folium.parentFolder().name() == L"Contours" ) || ( folium.parentFolder().name() == L"Spectrograms" ) ) {
        impl_->sgData_.reset();
        double zmax = 1.0;
        if ( portfolio::is_type< std::shared_ptr< const adcontrols::MassSpectra > >( folium ) ) {
            if ( auto map = portfolio::get< std::shared_ptr< const adcontrols::MassSpectra > >( folium ) ) {
                zmax = map->z_max();
                impl_->sgData_ = std::make_unique< SpectrogramData >( std::move( map ) );
            }
        } else if ( portfolio::is_type< std::shared_ptr< adcontrols::MassSpectra > >( folium ) ) {
            if ( auto map = portfolio::get< std::shared_ptr< adcontrols::MassSpectra > >( folium ) ) {
                zmax = map->z_max();
                impl_->sgData_ = std::make_unique< SpectrogramData >( std::move( map ) );
            }
        }
        if ( impl_->sgData_ ) {
            impl_->map_->setData( impl_->sgData_.get() );
            impl_->setImage( impl_->sgData_->matrix(), zmax );
        }
    }
}

QwtText
PlayerWnd::impl::make_map_title( const std::pair< double, double >& trig, const std::pair< double, double >& tof )
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
PlayerWnd::impl::make_spectrum_title( const std::pair< double, double >& trig )
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
PlayerWnd::impl::replotImage()
{
    ADDEBUG() << "-----> replot";

    if ( document::instance()->momentsEnabled() ) {
        
        // Contours requested
        // if ( auto image = image_.lock() )
        //     setImage( image->matrix(), zAutoScale_ ? image->max_z() : zScale_ );

    } else {

        // if ( auto spectra = spectra_.lock() ) { // lock 'mappedSpectra'
        //     if ( auto image = std::make_shared< adcontrols::MappedImage >() ) {
        //         if ( image->merge( *spectra ) ) // convert mappedSpectra to 2-D contour map
        //             setImage( image->matrix(), zAutoScale_ ? image->max_z() : zScale_ );
        //     }
        // }
    }
}

void
PlayerWnd::impl::setImage( const boost::numeric::ublas::matrix< double >& m, double maxScale )
{
    double scaleFactor = ( maxScale < std::numeric_limits<double>::epsilon() ) ? 1.0 : ( 1.0 / maxScale );
    QImage qImage;

    auto& method = document::instance()->contoursMethod();
    adcv::imBlur mBlur( { method.blurSize(), method.blurSize() }, method.sizeFactor() );

    if ( filters_ & 0x08 ) { // contours search
        auto& method = document::instance()->contoursMethod();
        qImage = adcv::imfilter< QImage
                                 , adcv::imContours>( adcv::imContours( method ))( m, scaleFactor );
        
    } else if ( filters_ == 0 )
        qImage = adcv::imfilter< QImage
                                 , adcv::imColorMap >()( m, scaleFactor );
    
    else if ( filters_ == 1 )
        qImage = adcv::imfilter< QImage
                                 , adcv::imGrayScale >()( m, scaleFactor );
    else if ( filters_ == 2 )
        qImage = adcv::imfilter< QImage
                                 , adcv::imColorMap
                                 , adcv::imDFT >()( m, scaleFactor );
    else if ( filters_ == 4 )
        qImage = adcv::imfilter< QImage
                                 , adcv::imColorMap
                                 , adcv::imBlur >( adcv::imColorMap(), mBlur )( m, scaleFactor );
    else if ( filters_ == ( 0x01 | 0x02 ) ) // Gray+DFT
        qImage = adcv::imfilter< QImage
                                 , adcv::imGrayScale, adcv::imDFT >()( m, scaleFactor );
    else if ( filters_ == ( 0x01 | 0x04 ) ) // Gray+Blur
        qImage = adcv::imfilter< QImage
                                 , adcv::imGrayScale, adcv::imBlur >(
                                     adcv::imGrayScale(), mBlur )( m, scaleFactor );
    else if ( filters_ == ( 0x02 | 0x04 ) ) // ColorMap+DFT+Blur
        qImage = adcv::imfilter< QImage
                                 , adcv::imColorMap, adcv::imDFT, adcv::imBlur >(
                                     adcv::imColorMap(), adcv::imDFT(), mBlur )( m, scaleFactor );
    else if ( filters_ == ( 0x01 | 0x02 | 0x04 ) ) // Gray+DFT+Blur
        qImage = adcv::imfilter< QImage
                                 , adcv::imGrayScale, adcv::imDFT, adcv::imBlur >(
                                     adcv::imGrayScale(), adcv::imDFT(), mBlur )( m, scaleFactor );
    try {
        img_->setImage( qImage );
    } catch ( std::exception& ex ) {
        ADDEBUG() << boost::diagnostic_information( ex );
    }
}

void
PlayerWnd::__setData( std::shared_ptr< const adcontrols::MappedSpectra >&& spectra, const std::pair< double, double >& trig )
{
#if 0
    QwtText title;

    impl_->spectra_ = spectra;
    impl_->tofRange_ = (*spectra)( 0, 0 ).acqTimeRange();

    if ( document::instance()->histogramWindowEnabled() ) {

        auto range = document::instance()->histogramWindow();

        if ( auto image = applyTofWindow( range, impl_->tofRange_ ) ) {

            document::instance()->setMappedImage( image, impl_->spectra_.lock(), trig, impl_->tofRange_ );
            impl_->setImage( image->matrix(), zMapScale( *image ) );

            // impl_->map_->setData( std::move( image ) );
            // impl_->map_->setTitle( impl::make_map_title( trig, impl_->tofRange_ ) );
        }

    } else {

        if ( auto image = std::make_shared< adcontrols::MappedImage >() ) {
            if ( image->merge( *spectra ) ) {
                document::instance()->setMappedImage( image, impl_->spectra_.lock(), trig, impl_->tofRange_ );
                impl_->setImage( image->matrix(), zMapScale( *image ) );

                // impl_->map_->setData( std::move( image ) );
                // impl_->map_->setTitle( impl::make_map_title( trig, impl_->tofRange_ ) );
            }
        }
    }

    const auto& tof = impl_->tofRange_;

    // plot spectrum
    if ( auto sp = std::make_unique< adcontrols::MappedSpectrum >() ) {
        spectra->sum_in_range( *sp, 0, 0, spectra->size1(), spectra->size2() );
        if ( auto ms = std::make_shared< adcontrols::MassSpectrum >() ) {
            if ( sp->transform( *ms ) ) {

                impl_->ms1_ = ms;
                impl_->ms2_.reset(); // clear cell selected spectrum

                impl_->splots_[ 0 ]->setData( ms, 0 );
                impl_->splots_[ 0 ]->setTitle( impl::make_spectrum_title( trig ).text() );

                impl_->tofRangeMarker_->setXValue( tof.first * std::micro::den, tof.second * std::micro::den );
                impl_->splots_[ 0 ]->replot();
            }

        }
    }

    impl_->timedPlotMarker_->setXValue( trig.first, trig.second );
    impl_->timed_plot_->replot();
#endif
}

void
PlayerWnd::__setData( std::shared_ptr< const adcontrols::MappedImage >&& image, const std::pair< double, double >& trig )
{
    //QwtText title;
    ADDEBUG() << __FUNCTION__;
    // impl_->image_ = image;
    // document::instance()->setMappedImage( image, nullptr, trig, impl_->tofRange_ );
    // impl_->setImage( image->matrix(), zMapScale( *image ) );
    //impl_->map_->setData( std::move( image ) );
    //impl_->map_->setTitle( impl::make_map_title( trig, impl_->tofRange_ ) );

    // impl_->timedPlotMarker_->setXValue( trig.first, trig.second );
    // impl_->timed_plot_->replot();
}

// from adplot::ChromatogramWidget
void
PlayerWnd::handleSelectedOnChromatogram( const QRectF& rc )
{
    impl_->avgCount_ = 0; // reset player setting

    if ( auto doc = document::instance()->currentProcessor() ) {

        // auto range = std::make_pair( doc->findElapsedTime( rc.left() ), doc->findElapsedTime( rc.right() ) );

        // if ( document::instance()->momentsEnabled() ) {
        //     if ( auto img = doc->mappedImage( range.first, range.second, document::instance()->contoursMethod() ) ) {
        //         auto spectra =
        //         impl_->trigRange_ = std::make_pair( range.first->time_since_inject(), range.second->time_since_inject() );
        //         __setData( std::move( img ), impl_->trigRange_ );

        //         emit frameChanged( range.first->time_since_inject(), range.first->pos(), ( range.second->pos() - range.first->pos() ) );
        //     }
        // } else {

        //     if ( auto matrix = doc->mappedSpectra( range.first, range.second ) ) {

        //         impl_->trigRange_ = std::make_pair( range.first->time_since_inject(), range.second->time_since_inject() );

        //         __setData( std::move( matrix ), impl_->trigRange_ );

        //         emit frameChanged( range.first->time_since_inject(), range.first->pos(), ( range.second->pos() - range.first->pos() ) );
        //     }
        // }

    }
}

void
PlayerWnd::handleNextMappedSpectra( bool forward )
{
    if ( auto doc = document::instance()->currentProcessor() ) {

        // DataReader::iterator range
    //     auto range = doc->selectedRangeOnTimedTrace();
    //     int avgCount = int ( impl_->avgCount_ );

    //     if ( avgCount == 0 )
    //         avgCount = std::max( 1, int( range.second->pos() - range.first->pos() ) ); // 0..n

    //     if ( forward ) {
    //         std::advance( range.first, avgCount );
    //         range.second = std::next( range.first, avgCount );
    //     } else {
    //         std::advance( range.second, - avgCount );
    //         range.first = std::prev( range.first, avgCount );
    //     }

    //     if ( range.first->rowid() == (-1) || range.second->rowid() == (-1) ) {
    //         ADDEBUG() << "------------- EOF ---------------";
    //         document::instance()->player()->Stop();
    //         emit endOfFile();
    //         return;
    //     }

    //     if ( auto matrix = doc->mappedSpectra( range.first, range.second ) ) {

    //         QRectF base = impl_->timed_plot_->zoomer()->zoomBase();
    //         QRectF rc = impl_->timed_plot_->zoomRect();

    //         if ( base.width() / 2 > rc.width() ) {

    //             double left = range.first->time_since_inject() - rc.width() / 2;
    //             impl_->timed_plot_->zoomer()->moveTo( QPointF( left, 0.0 ) );

    //         }

    //         impl_->trigRange_ = { range.first->time_since_inject(), range.second->time_since_inject() };

    //         __setData( std::move( matrix ), impl_->trigRange_ );

    //         emit frameChanged( range.first->time_since_inject(), range.first->pos(), ( range.second->pos() - range.first->pos() ) );
    //     }
    }
}

void
PlayerWnd::handleTofMoved( int step )
{
    //using namespace adcontrols::metric;
    // if ( auto matrix = impl_->spectra_.lock() ) {

    //     const auto& sp = (*matrix)( 0, 0 );
    //     double distance = sp.samplingInterval() * step;
    //     auto acqRaneg = sp.acqTimeRange();

    //     std::pair< double, double > tof = { impl_->tofRange_.first + distance, impl_->tofRange_.second + distance };

    //     if ( ( step > 0 ) && ( tof.second > acqRaneg.second ) ) // over range
    //         return;
    //     else if ( tof.first < acqRaneg.first ) // under range
    //         return;

    //     impl_->tofRange_ = tof;

    //     double ctof  = tof.first + ( tof.second - tof.first ) / 2;
    //     double width = tof.second - tof.first;

    //     if ( auto image = std::make_shared< adcontrols::MappedImage >() ) {

    //         if ( image->merge( *matrix, ctof, width ) ) {

    //             //////////////////////
    //             impl_->setImage( image->matrix(), zMapScale( *image ) );
    //             //impl_->map_->setData( std::move( image ) );
    //             //impl_->map_->setTitle( impl::make_map_title( impl_->trigRange_, tof ) );

    //             QRectF rc = impl_->splots_[ 0 ]->zoomRect();
    //             QRectF tofRect( scale_to_micro( tof.first ), rc.y(), scale_to_micro( tof.second - tof.first ), rc.height() );

    //             if ( rc.right() < tofRect.right() ) {
    //                 impl_->splots_[ 0 ]->zoomer()->moveBy( tofRect.right() - rc.right(), 0 );
    //             } else if ( rc.left() > tofRect.left() ) {
    //                 impl_->splots_[ 0 ]->zoomer()->moveBy( tofRect.left() - rc.left(), 0 );
    //             }

    //             impl_->tofRangeMarker_->setXValue( scale_to_micro( tof.first ), scale_to_micro( tof.second ) );
    //             impl_->splots_[ 0 ]->replot();
    //         }
    //         document::instance()->setHistogramWindow( tof.first, tof.second - tof.first ); // delay, window
    //     }
    // }
}

std::shared_ptr< adcontrols::MappedImage >
PlayerWnd::applyTofWindow( const std::pair< double, double >& range, std::pair< double, double >& tofRange ) const
{
    // auto image = std::make_shared< adcontrols::MappedImage >();

    // if ( auto matrix = impl_->spectra_.lock() ) {

    //     const auto& sp = (*matrix)( 0, 0 );

    //     // if ( document::instance()->histogramWindowEnabled() ) {

    //     double lower = std::max( range.first, sp.acqTimeRange().first );
    //     double upper = std::min( range.first + range.second, sp.acqTimeRange().second );

    //     tofRange = { lower, upper };

    //     double tof = lower + ( upper - lower ) / 2.0;
    //     double width = ( upper - lower );

    //     if ( image->merge( *matrix, tof, width ) )
    //         return image;
    // }
    return nullptr;
}

void
PlayerWnd::handleSelectedOnSpectrum( const QRectF& rc )
{
}

void
PlayerWnd::handleAxisChanged()
{
}

void
PlayerWnd::handleCellSelected( const QRect& rc )
{
    impl_->cellSelected_ = rc;
}

void
PlayerWnd::handleCellMoved( int hor, int vert )
{
    QRect rc( impl_->cellSelected_ );

    rc.moveTo( rc.left() + hor, rc.top() + vert );
    QRect full( 0, 0, 64, 64 );

    if ( full.contains( rc ) ) {
        // QSignalBlocker block( impl_->map_.get() );
        // impl_->map_->setCellSelection( rc );
        handleCellSelected( rc );
    }
}

void
PlayerWnd::setTimeRangeOnSpectrum( double tof, double window )
{
    QRectF rc( ( tof - window / 2.0 ) * std::micro::den, 0, ( tof + window / 2.0 ) * std::micro::den, 0 );
    handleSelectedOnSpectrum( rc );
}

void
PlayerWnd::handlePlayerSignal()
{
    handleNextMappedSpectra( true ); // forward
    QCoreApplication::processEvents();  // avoid block UI
    document::instance()->player()->signal(); // next
}

void
PlayerWnd::handleHistogramClearCycleChanged( int value )
{
    impl_->avgCount_ = value;

    if ( impl_->avgCount_ > 0 )
        document::instance()->player()->setAverageCount( impl_->avgCount_ );

    // if ( ! impl_->zAutoScale_ )
    //     impl_->map_->setAxisZMax( value );
}

void
PlayerWnd::handleTofWindowChanged()
{
#if 0
    if ( auto matrix = impl_->spectra_.lock() ) {

        const auto& sp = (*matrix)( 0, 0 );

        if ( document::instance()->histogramWindowEnabled() ) {
            auto range = document::instance()->histogramWindow(); // delay, width
            double lower = std::max( range.first, sp.acqTimeRange().first );
            double upper = std::min( range.first + range.second, sp.acqTimeRange().second );
            impl_->tofRange_ = std::make_pair( lower, upper );
        } else {
            impl_->tofRange_ = sp.acqTimeRange(); // set full tof range
        }
        setData( std::move( matrix ), impl_->trigRange_, impl_->tofRange_ );
    }
#endif
}


void
PlayerWnd::setFilter( unsigned int id, bool enable ) // 1 = Gray, 2 = DFT, 4 = Blur, 8 = Contours
{
    auto prev = impl_->filters_;
    if ( enable )
        impl_->filters_ |= id;
    else
        impl_->filters_ ^= id;

    if ( prev != impl_->filters_ )
        impl_->replotImage();
}

void
PlayerWnd::setZAutoScale( ZMapId id, bool checked )
{
    if ( id == zMapManip ) {

        impl_->zAutoScale_ = checked;
        impl_->replotImage();

    } else if ( id == zMapRaw ) {

        // if ( auto mappedSpectra = impl_->spectra_.lock() )
        //     impl_->map_->setAxisZMax( mappedSpectra->averageCount() );

    }
}

void
PlayerWnd::setZScale( ZMapId id, int scale )
{
    if ( id == zMapManip ) {

        impl_->zScale_ = scale;

        if ( ! impl_->zAutoScale_ )
            impl_->replotImage();

    } else if ( id == zMapRaw ) {

        //impl_->map_->setAxisZMax( scale );

    }
}


double
PlayerWnd::zMapScale( const adcontrols::MappedImage& img ) const
{
    //return document::instance()->zAutoScaleEnable( zMapManip ) ? img.max_z() : impl_->zScale_;
    return impl_->zScale_;
}

void
PlayerWnd::handleContoursMethodChanged()
{
    if ( impl_->filters_ & (0x08|0x04) ) { // Contours|Blur enabled
        impl_->replotImage();
    }
}

void
PlayerWnd::print( QPainter& painter, QPrinter& printer )
{
    // QRectF drawRect( 0.0, 0.0, printer.width(), printer.height() );
    // QRectF headRect( 0.0, 0.0, printer.width(), printer.height() * 0.1 );

    // boost::filesystem::path path = document::instance()->currentProcessor()->dataprocessor()->filename();

    // painter.drawText( drawRect, Qt::TextWordWrap, QString::fromStdString( path.string() ), &headRect );

    // QRectF rc0( 0.0,            headRect.bottom(), printer.width() / 2, ( printer.height() / 2 ) - headRect.height() ); // top
    // QRectF rc10( 0.0,           rc0.bottom(), printer.width() / 2, printer.height() / 2 - headRect.height() ); // bottom left
    // QRectF rc11( rc10.right(),  rc0.bottom(), printer.width() / 2, printer.height() / 2 - headRect.height()); // bottom right

    // impl_->img_->graphicsView()->render( &painter, rc10 );

    // QwtPlotRenderer renderer;
    // renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasBackground, true );
    // renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasFrame, true );
    // renderer.setDiscardFlag( QwtPlotRenderer::DiscardBackground, true );

    // renderer.render( impl_->splots_[ 0 ].get(), &painter, rc0 );

    // renderer.render( impl_->map_.get(), &painter, rc11 );
}
