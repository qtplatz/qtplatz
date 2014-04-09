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

#include "dataplot.hpp"
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
#include <QSvgGenerator>

using namespace adwplot;

Dataplot::~Dataplot()
{
	unlink();
}

Dataplot::Dataplot(QWidget *parent) : QwtPlot(parent)
                                    , linkedzoom_inprocess_( false )
{
    setCanvasBackground( QColor( "#d0d0d0" ) );

    // zoomer
    zoomer1_.reset( new Zoomer( int(QwtPlot::xBottom), int(QwtPlot::yLeft), canvas() ) );

    // picker
    picker_.reset( new Picker( canvas() ) );

    // panner
    panner_.reset( new Panner( canvas() ) );
    panner_->setMouseButton( Qt::LeftButton, Qt::AltModifier );
}

void
Dataplot::setTitle( const std::wstring& text )
{
    std::string utf8 = adportable::utf::to_utf8( text );
    setTitle( utf8 );
}

void
Dataplot::setTitle( const std::string& text )
{
	QwtText qwtText( text.c_str(), QwtText::RichText );
    QFont font = qwtText.font();

    qtwrapper::font::setFont( font, qtwrapper::fontSizePlotTitle, qtwrapper::fontPlotTitle );
    qwtText.setFont( font );
    qwtText.setRenderFlags( Qt::AlignLeft | Qt::AlignTop );

    QwtPlot::setTitle( qwtText );
}

void
Dataplot::setFooter( const std::wstring& text )
{
    std::string utf8 = adportable::utf::to_utf8( text );
    setFooter( utf8 );
}

void
Dataplot::setFooter( const std::string& text )
{
	QwtText qwtText( text.c_str(), QwtText::RichText );
    QFont font = qwtText.font();
    qwtText.setFont( qtwrapper::font::setFont( font, qtwrapper::fontSizePlotFooter, qtwrapper::fontPlotFooter ) );
    qwtText.setRenderFlags( Qt::AlignRight | Qt::AlignBottom );
    QwtPlot::setFooter( qwtText );
}

QRectF
Dataplot::zoomRect() const
{
	return zoomer1_->zoomRect();
}

//virtual method
void
Dataplot::zoom( const QRectF& rect )
{
	zoomer1_->zoom( rect );
}

// private
void
Dataplot::zoom( const QRectF& rect, const Dataplot& origin )
{
	if ( this == &origin )
		return;
	linkedzoom_inprocess_ = true;
	zoomer1_->zoom( rect ); // will emit onZoomed
	linkedzoom_inprocess_ = false;
}

void
Dataplot::panne( int dx, int dy, const Dataplot& origin )
{
	if ( this != &origin )
		panner_->panne( dx, dy );
}

//virtual slot
void
Dataplot::onZoomed( const QRectF& rect )
{
	if ( plotlink_ && ! linkedzoom_inprocess_ ) {
		for ( plotlink::value_type plot: *plotlink_ ) {
			plot->zoom( rect, *this );
		}
	}
}

//virtual slot
void
Dataplot::onPanned( int dx, int dy )
{
	if ( plotlink_ && ! linkedzoom_inprocess_ ) {
		for ( plotlink::value_type plot: *plotlink_ ) {
			plot->panne( dx, dy, *this );
		}
	}
}

void
Dataplot::link( Dataplot * p )
{
	if ( ! plotlink_ ) {
		connect( zoomer1_.get(), SIGNAL( zoomed( const QRectF& ) ), this, SLOT( onZoomed( const QRectF& ) ) );
		connect( panner_.get(), SIGNAL( panned( int, int ) ), this, SLOT( onPanned( int, int ) ) );
	}

	if ( plotlink_ && p->plotlink_ ) {
		// marge into this->plotlink_
		plotlink_->insert( plotlink_->end(), p->plotlink_->begin(), p->plotlink_->end() );

		// update all plotlink in the marged list
		for ( plotlink::iterator it = p->plotlink_->begin(); it != p->plotlink_->end(); ++it )
			(*it)->plotlink_ = plotlink_;

	} else if ( plotlink_ && ! p->plotlink_ ) {

		p->plotlink_ = plotlink_;
		if ( std::find( plotlink_->begin(), plotlink_->end(), p ) == plotlink_->end() )
			plotlink_->push_back( p );

	} else if ( ! plotlink_ && p->plotlink_ ) {

		plotlink_ = p->plotlink_;
		if ( std::find( plotlink_->begin(), plotlink_->end(), this ) == plotlink_->end() )
			plotlink_->push_back( this );

	} else if ( ! plotlink_ && ! p->plotlink_ ) {
		plotlink_.reset( new plotlink );
		plotlink_->push_back( this );
		plotlink_->push_back( p );
	}
}

void
Dataplot::unlink()
{
	if ( plotlink_ ) {
		disconnect( zoomer1_.get(), SIGNAL( zoomed( const QRectF& ) ), this, SLOT( onZoomed( const QRectF& ) ) );
		disconnect( panner_.get(), SIGNAL( panned( int, int ) ), this, SLOT( onPanned( int, int ) ) );
		plotlink_->erase( std::remove( plotlink_->begin(), plotlink_->end(), this ) );
		plotlink_.reset();
	}
}

//static
void
Dataplot::copyToClipboard( Dataplot * plot )
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
Dataplot::copyImageToFile( Dataplot * plot, const QString& file, const char * format )
{
    QwtPlotRenderer renderer;

    renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasBackground, true );
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasFrame, true );
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardBackground, true );

    renderer.renderDocument( plot, file, format, QSizeF( 320, 80 ), 150 );
}

