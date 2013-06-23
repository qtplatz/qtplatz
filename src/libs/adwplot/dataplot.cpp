// -*- C++ -*-
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

#include "dataplot.hpp"
#include "annotation.hpp"
#include "annotations.hpp"
#include "trace.hpp"
#include "traces.hpp"
#include "zoomer.hpp"
#include "picker.hpp"
#include "panner.hpp"
#include <qwt_picker_machine.h>
#include <qwt_scale_widget.h>
#include <qtwrapper/qstring.hpp>
#include <algorithm>
#include <boost/foreach.hpp>

using namespace adwplot;

Dataplot::~Dataplot()
{
	unlink();
}

Dataplot::Dataplot(QWidget *parent) : QwtPlot(parent)
                                    , linkedzoom_inprocess_( false )
{
  setCanvasBackground( QColor( Qt::lightGray ) );

  // zoomer
#if QWT_VERSION >= 0x060100
  zoomer1_.reset( new Zoomer( int(QwtPlot::xBottom), int(QwtPlot::yLeft), canvas() ) );
#else // 0x060003 or earlier
  zoomer1_.reset( new Zoomer( QwtPlot::xBottom, QwtPlot::yLeft, canvas() ) );
#endif

  // zoomer2_.reset( new Zoomer( QwtPlot::xTop, QwtPlot::yRight, canvas() ) );

  // picker
  picker_.reset( new Picker( canvas() ) );
  // picker_->setStateMachine( new QwtPickerDragPointMachine() );
  // panner
  panner_.reset( new Panner( canvas() ) );
  panner_->setMouseButton( Qt::LeftButton, Qt::AltModifier );
}

void
Dataplot::setTitle( const std::wstring& title )
{
    QwtPlot::setTitle( qtwrapper::qstring( title ) );
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
		BOOST_FOREACH( plotlink::value_type plot, *plotlink_ ) {
			plot->zoom( rect, *this );
		}
	}
}

//virtual slot
void
Dataplot::onPanned( int dx, int dy )
{
	if ( plotlink_ && ! linkedzoom_inprocess_ ) {
		BOOST_FOREACH( plotlink::value_type plot, *plotlink_ ) {
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
	// size_t n = plotlink_->size();
	// connect( zoomer1_.get(), SIGNAL( zoomed( const QRectF& ) ), p, SLOT( zoom( const QRectF& ) ) );
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

