// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC
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

#include "boundingrect.hpp"
#include <QFontMetrics>
#include <QPointF>
#include <QRectF>
#include <QSizeF>
#include <QtGlobal>
#include <qwt_plot.h>
#include <qwt_plot_marker.h>
#include <qwt_scale_map.h>
#include <qwt_text.h>

namespace adplot {

    QRectF boundingRect::operator()( QwtPlot& plot, double x, double y, const QwtText& label, Qt::Alignment align ) const {
        return (*this)( plot, QPointF{ x, y }, label, align );
    }

    QRectF boundingRect::operator()( QwtPlot& plot, QPointF xy, const QwtText& label, Qt::Alignment align ) const {
        const QwtScaleMap xMap = plot.canvasMap( QwtPlot::xBottom );
        const QwtScaleMap yMap = plot.canvasMap( QwtPlot::yLeft );
        QPointF pt( QwtScaleMap::transform( xMap, yMap, xy ) );

        QRectF rc = QwtScaleMap::transform( xMap, yMap, QRectF( pt, label.textSize() ));

        qreal xoffs{0}, yoffs{0};
        if ( align & Qt::AlignLeft ) {
            xoffs = 0;
        } else if ( align & Qt::AlignHCenter ) {
            xoffs = -rc.width() / 2;
        } else if ( align & Qt::AlignRight ) {
            xoffs = -rc.width();
        }

        if ( align & Qt::AlignTop ) {
            yoffs = 0;
        } else if ( align & Qt::AlignVCenter ) {
            yoffs = -rc.height() / 2;
        } else if ( align & Qt::AlignBottom ) {
            yoffs = -rc.height();
        }
        rc.moveTo( rc.x() + xoffs, rc.y() + yoffs );
        return rc;
    }

}
