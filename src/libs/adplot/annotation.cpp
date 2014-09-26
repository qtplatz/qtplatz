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

#include "annotation.hpp"
#include "plot.hpp"
#include <qtwrapper/qstring.hpp>
#include <qwt_plot_marker.h>
#include <qwt_text.h>
#include <memory>

namespace adplot {

    class Annotation::impl {
    public:
        impl( adplot::plot * p ) : plot_( p )
                                 , marker_( std::make_shared< QwtPlotMarker >()  ) {
        }
        impl( const impl& t ) : plot_( t.plot_ )
                              , marker_( t.marker_ ) {
        }
        adplot::plot * plot_;
        std::shared_ptr< QwtPlotMarker > marker_;
    };

}

using namespace adplot;

Annotation::~Annotation()
{
    delete impl_;
}

Annotation::Annotation( plot& plot
                        , const std::wstring& label
                        , double x, double y
                        , Qt::GlobalColor color ) : impl_( new impl( &plot ) )
{
    impl_->marker_->setValue( x, y );
    impl_->marker_->setLineStyle( QwtPlotMarker::NoLine );
    impl_->marker_->setLabelAlignment( Qt::AlignRight | Qt::AlignBottom );
    QwtText text( qtwrapper::qstring::copy( label ) );
    text.setFont( font() );
    text.setColor( color );
    impl_->marker_->setLabel( text );
    impl_->marker_->attach( &plot );
}

Annotation::Annotation( plot& plot
                        , const QwtText& label
                        , const QPointF& xy
                        , Qt::Alignment align ) : impl_( new impl( &plot ) )
{
    impl_->marker_->setValue( xy );
    impl_->marker_->setLineStyle( QwtPlotMarker::NoLine );
    impl_->marker_->setLabelAlignment( align );
    impl_->marker_->setLabel( label );
    impl_->marker_->attach( &plot );
}

Annotation::Annotation( const Annotation& t ) : impl_( new impl( *t.impl_ ) )
{
} 

void
Annotation::setLabelAlighment( Qt::Alignment align )
{
    impl_->marker_->setLabelAlignment( align );
}

QwtPlotMarker *
Annotation::getPlotMarker() const
{
    return impl_->marker_.get();
}

QFont
Annotation::font()
{
    return QFont( "Calibri", 9, QFont::Normal );
}
