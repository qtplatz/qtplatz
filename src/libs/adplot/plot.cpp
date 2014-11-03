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

#include "plot.hpp"
#include "annotation.hpp"
#include "annotations.hpp"
#include "trace.hpp"
#include "traces.hpp"
#include "zoomer.hpp"
#include "picker.hpp"
#include "panner.hpp"
#include <qtwrapper/font.hpp>
#include <qwt_picker_machine.h>
#include <qwt_scale_widget.h>
#include <adportable/utf.hpp>
#include <algorithm>
#include <qwt_plot_renderer.h>
#include <QApplication>
#include <QClipboard>
//#include <QSvgGenerator>

namespace adplot {

    class plot::impl {
    public:
        ~impl() {
        }
        impl( plot * pThis ) : this_( pThis ) 
                             , linkedzoom_inprocess_( false )
                             , vectorCompression_( false ) 
                             , zoomer1_( new Zoomer( int(QwtPlot::xBottom), int(QwtPlot::yLeft), pThis->canvas() ) )
                             , picker_( new Picker( pThis->canvas() ) )
                             , panner_( new Panner( pThis->canvas() ) )  {

            panner_->setMouseButton( Qt::LeftButton, Qt::AltModifier );

        }
        
        void zoom( const QRectF& rect, const plot& origin ) {
            if ( this_ == &origin )
                return;
            linkedzoom_inprocess_ = true;
            zoomer1_->zoom( rect ); // will emit onZoomed
            linkedzoom_inprocess_ = false;
        }
        
        void panne( int dx, int dy, const plot& origin ) {
            if ( this_ != &origin )
                panner_->panne( dx, dy );
        }

        plot * this_;
        typedef std::vector<plot *> plotlink;
        std::shared_ptr< plotlink > plotlink_;
        bool linkedzoom_inprocess_;
        bool vectorCompression_;
        std::unique_ptr< Zoomer > zoomer1_;  // left bottom axix
        std::unique_ptr< Picker > picker_;   // (right mouse button)
        std::unique_ptr< Panner > panner_;
    };

}

using namespace adplot;

plot::~plot()
{
	unlink();
}

plot::plot(QWidget *parent) : QwtPlot(parent)
                            , impl_(new impl(this))
{
    setCanvasBackground( QColor( "#d0d0d0" ) );
}

void
plot::setTitle( const QString& text )
{
	QwtText qwtText( text, QwtText::RichText );
    QFont font = qwtText.font();

    qtwrapper::font::setFont( font, qtwrapper::fontSizePlotTitle, qtwrapper::fontPlotTitle );
    qwtText.setFont( font );
    qwtText.setRenderFlags( Qt::AlignLeft | Qt::AlignTop );

    QwtPlot::setTitle( qwtText );
}

void
plot::setFooter( const QString& text )
{
	QwtText qwtText( text, QwtText::RichText );
    QFont font = qwtText.font();
    qwtText.setFont( qtwrapper::font::setFont( font, qtwrapper::fontSizePlotFooter, qtwrapper::fontPlotFooter ) );
    qwtText.setRenderFlags( Qt::AlignRight | Qt::AlignBottom );
    QwtPlot::setFooter( qwtText );
}

void
plot::setVectorCompression( int compression )
{
    impl_->vectorCompression_ = compression;
}

int
plot::vectorCompression() const
{
    return impl_->vectorCompression_;
}

QRectF
plot::zoomRect() const
{
    return impl_->zoomer1_->zoomRect();
}

//virtual method
void
plot::zoom( const QRectF& rect )
{
	impl_->zoomer1_->zoom( rect );
}

//virtual slot
void
plot::onZoomed( const QRectF& rect )
{
    if ( impl_->plotlink_ && !impl_->linkedzoom_inprocess_ ) {
        for ( impl::plotlink::value_type plot : *impl_->plotlink_ ) {
            plot->impl_->zoom( rect, *this );
		}
	}
}

//virtual slot
void
plot::onPanned( int dx, int dy )
{
    if ( impl_->plotlink_ && !impl_->linkedzoom_inprocess_ ) {
        for ( impl::plotlink::value_type plot : *impl_->plotlink_ ) {
            plot->impl_->panne( dx, dy, *this );
		}
	}
}

void
plot::link( plot * p )
{
    if ( !impl_->plotlink_ ) {
        connect( plot::zoomer(), SIGNAL( zoomed( const QRectF& ) ), this, SLOT( onZoomed( const QRectF& ) ) );
		connect( plot::panner(), SIGNAL( panned( int, int ) ), this, SLOT( onPanned( int, int ) ) );
	}

    if ( impl_->plotlink_ && p->impl_->plotlink_ ) {
		// marge into this->plotlink_
        impl_->plotlink_->insert( impl_->plotlink_->end(), p->impl_->plotlink_->begin(), p->impl_->plotlink_->end() );

		// update all plotlink in the marged list
        for ( impl::plotlink::iterator it = p->impl_->plotlink_->begin(); it != p->impl_->plotlink_->end(); ++it )
            (*it)->impl_->plotlink_ = impl_->plotlink_;

    }
    else if ( impl_->plotlink_ && !p->impl_->plotlink_ ) {

        p->impl_->plotlink_ = impl_->plotlink_;
        if ( std::find( impl_->plotlink_->begin(), impl_->plotlink_->end(), p ) == impl_->plotlink_->end() )
            impl_->plotlink_->push_back( p );

    }
    else if ( !impl_->plotlink_ && p->impl_->plotlink_ ) {

        impl_->plotlink_ = p->impl_->plotlink_;
        if ( std::find( impl_->plotlink_->begin(), impl_->plotlink_->end(), this ) == impl_->plotlink_->end() )
            impl_->plotlink_->push_back( this );

    }
    else if ( !impl_->plotlink_ && !p->impl_->plotlink_ ) {
        impl_->plotlink_ = std::make_shared< impl::plotlink >();
        impl_->plotlink_->push_back( this );
        impl_->plotlink_->push_back( p );
	}
}

void
plot::unlink()
{
	if ( impl_->plotlink_ ) {
		disconnect( zoomer(), SIGNAL( zoomed( const QRectF& ) ), this, SLOT( onZoomed( const QRectF& ) ) );
		disconnect( panner(), SIGNAL( panned( int, int ) ), this, SLOT( onPanned( int, int ) ) );
        impl_->plotlink_->erase( std::remove( impl_->plotlink_->begin(), impl_->plotlink_->end(), this ) );
        impl_->plotlink_.reset();
	}
}

//static
void
plot::copyToClipboard( plot * plot )
{
    //QRectF rc = plot->zoomRect();
    QImage img( plot->size(), QImage::Format_ARGB32 );
    QPainter painter(&img);

    QwtPlotRenderer renderer;
    renderer.render( plot, &painter, plot->rect() );
    if ( QClipboard * clipboard = QApplication::clipboard() )
        clipboard->setImage( img );
}

//static
void
plot::copyImageToFile( plot * plot, const QString& file, const QString& format, bool compress, int dpi )
{
    QwtPlotRenderer renderer;

    renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasBackground, true );
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasFrame, true );
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardBackground, true );

    plot->setVectorCompression( compress );

    renderer.renderDocument( plot, file, format, QSizeF( 210.0 * 0.9, 80 ), dpi );

    plot->setVectorCompression( 0 );
}

Zoomer *
plot::zoomer( int idx ) const
{
    if ( idx == 0  )
        return impl_->zoomer1_.get();
    return 0;
}

Picker * 
plot::picker() const
{
    return impl_->picker_.get();
}

Panner *
plot::panner() const
{
    return impl_->panner_.get();
}
