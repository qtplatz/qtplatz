/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "spectrogramwnd.hpp"
#include "sessionmanager.hpp"
#include <adcontrols/massspectra.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adportable/debug.hpp>
#include <adportable/array_wrapper.hpp>
#include <portfolio/folium.hpp>
#include <portfolio/folder.hpp>
#include <adwplot/spectrogramwidget.hpp>
#include <adwplot/spectrogramdata.hpp>
#include <qwt_plot_renderer.h>
#include <QPrinter>
#include <QPrintDialog>
#include <QBoxLayout>
#include <boost/numeric/ublas/matrix.hpp>
#include <algorithm>

namespace dataproc {
    namespace detail {

        class SpectrogramData : public adwplot::SpectrogramData {
        public:
            SpectrogramData( std::shared_ptr< adcontrols::MassSpectra >& spectra );
            SpectrogramData( const SpectrogramData& );
            double value( double x, double y ) const override;
            QRectF boundingRect() const override;
            bool zoomed( const QRectF& ) override;
            
        private:
            std::shared_ptr< adcontrols::MassSpectra > spectra_;
            boost::numeric::ublas::matrix< double > m_;
            std::pair< double, double > xlimits_, ylimits_;
            size_t dx( double x ) const;
            size_t dy( double y ) const;
            void updateData();
            size_t size1_;
            size_t size2_;
        };

    }
}

using namespace dataproc;

SpectrogramWnd::SpectrogramWnd(QWidget *parent) : QWidget(parent)
                                                , plot_( std::make_shared< adwplot::SpectrogramWidget >() )
{
    init();
}

void
SpectrogramWnd::init()
{
    QBoxLayout * layout = new QVBoxLayout( this );
    layout->addWidget( plot_.get() );
}

void
SpectrogramWnd::handlePrintCurrentView( const QString& pdfname )
{
	// A4 := 210mm x 297mm (8.27 x 11.69 inch)

    QPrinter printer( QPrinter::HighResolution );

    printer.setOutputFileName( pdfname );
    printer.setOrientation( QPrinter::Landscape );
    printer.setDocName( "QtPlatz Spectrogram" );
    printer.setColorMode( QPrinter::Color );
    printer.setPaperSize( QPrinter::A4 );
    printer.setFullPage( false );
    printer.setResolution( 300 );

    QPainter painter( &printer );
    QRectF drawRect( printer.resolution()/2, printer.resolution()/2, printer.width() - printer.resolution(), (12.0/72)*printer.resolution() );
    
    QRectF boundingRect;
    printer.setDocName( "QtPlatz Process Report" );
    painter.drawText( drawRect, Qt::TextWordWrap, fullpath_.c_str(), &boundingRect );

    drawRect.setTop( boundingRect.bottom() + printer.resolution() / 4 );
    drawRect.setHeight( printer.height() - boundingRect.top() - printer.resolution()/2 );
    // drawRect.setWidth( printer.width() );
    
    QwtPlotRenderer renderer;
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardBackground, true );
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasBackground, true );
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasFrame, true );
    
    if ( printer.colorMode() == QPrinter::GrayScale )
        renderer.setLayoutFlag( QwtPlotRenderer::FrameWithScales );
    
    renderer.render( plot_.get(), &painter, drawRect );
}

void
SpectrogramWnd::handleSessionAdded( Dataprocessor* )
{
}

void
SpectrogramWnd::handleSelectionChanged( Dataprocessor*, portfolio::Folium& folium )
{
    portfolio::Folder folder = folium.getParentFolder();

    if ( folder && folder.name() == L"Spectrograms" ) {
        adcontrols::MassSpectraPtr ptr;
        if ( portfolio::Folium::get< adcontrols::MassSpectraPtr >( ptr, folium ) ) {
            foliumId_ = folium.id();
            fullpath_ = folium.fullpath();
            plot_->setData( new detail::SpectrogramData( ptr ) );
        }
    }
}

void
SpectrogramWnd::handleApplyMethod( const adcontrols::ProcessMethod& )
{
}

void
SpectrogramWnd::handleCheckStateChanged( Dataprocessor*, portfolio::Folium&, bool )
{
}

//
namespace dataproc {
    namespace detail {

        SpectrogramData::SpectrogramData( const SpectrogramData& t ) : spectra_( t.spectra_ )
                                                                     , m_( t.m_ )
                                                                     , xlimits_( t.xlimits_ )
                                                                     , ylimits_( t.ylimits_ )
                                                                     , size1_( t.size1_ )
                                                                     , size2_( t.size2_ )
        {
        }

        SpectrogramData::SpectrogramData( adcontrols::MassSpectraPtr& spectra ) : spectra_( spectra )
                                                                                , m_( 1280, 720 ) // 720p
                                                                                , xlimits_( spectra_->x_left(), spectra_->x_right() )
                                                                                , ylimits_( spectra_->lower_mass(), spectra_->upper_mass() )
        {
            size1_ = m_.size1();
            size2_ = m_.size2();
            updateData();
        }

        size_t 
        SpectrogramData::dx( double x ) const
        {
            //size_t d = ((x - xlimits_.first) / ( xlimits_.second - xlimits_.first )) * ( m_.size1() - 1 );
            size_t d = ((x - xlimits_.first) / ( xlimits_.second - xlimits_.first )) * ( size1_ - 1 );
			if ( d > m_.size1() - 1 )
				return m_.size1() - 1;
			return d;
        }

        size_t
        SpectrogramData::dy( double y ) const
        {
            size_t d = ((y - ylimits_.first) / ( ylimits_.second - ylimits_.first )) * ( m_.size2() - 1 );
			if ( d > m_.size2() - 1 )
				return m_.size2() - 1;
            return d;
        }
        
        double
        SpectrogramData::value( double x, double y ) const
        {
			size_t ix = dx( x );
			size_t iy = dy( y );
            return m_( ix, iy );
        }

        QRectF
        SpectrogramData::boundingRect() const
        {
            return QRectF( xlimits_.first, ylimits_.first, xlimits_.second - xlimits_.first, ylimits_.second - ylimits_.first );
        }

        bool
        SpectrogramData::zoomed( const QRectF& rc ) 
        {
            xlimits_ = std::make_pair( rc.left(), rc.right() );
            ylimits_ = std::make_pair( rc.top(), rc.bottom() );
            updateData();
            return true;
        }

        void
        SpectrogramData::updateData() 
        {
            for ( size_t i = 0; i < m_.size1(); ++i ) {
                for ( size_t j = 0; j < m_.size2(); ++j ) {
                    m_( i, j ) = 0;
                }
            }
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
                        for ( int i = 0; i < seg.size(); ++i ) {
                            double m = seg.getMass( i );
                            if ( ylimits_.first < m && m < ylimits_.second ) {
                                size_t iy = dy(m);
                                m_( ix, iy ) += seg.getIntensity( i ); 
                                z_max = std::max( z_max, m_( ix, iy ) );
                            }
                        }
                    }
                }
            }
            // for ( size_t i = 0; i < m_.size1(); ++i ) {
            //     for ( size_t j = 0; j < m_.size2(); ++j ) {
            //         m_( i, j ) /= (z_max / 10000.0);
            //     }
            // }
            setInterval( Qt::XAxis, QwtInterval( spectra_->x_left(), spectra_->x_right() ) );   // time (sec -> min)
            setInterval( Qt::YAxis, QwtInterval( spectra_->lower_mass(), spectra_->upper_mass() ) ); // m/z
            setInterval( Qt::ZAxis, QwtInterval( 0.0, z_max ) );
        }
    }
}

