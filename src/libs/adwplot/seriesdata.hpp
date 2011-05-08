// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

#pragma once

#include <qwt_series_data.h>

class QPointF;

namespace adcontrols { class Trace; class Chromatogram; class MassSpectrum; }

namespace adwplot {

    class SeriesData : public QwtSeriesData<QPointF> {
    public:
        virtual ~SeriesData() {}
        SeriesData();
        SeriesData( const SeriesData& );

        // implements QwtSeriesData<>
        virtual size_t size() const;
        virtual QPointF sample( size_t idx ) const;
        virtual QRectF boundingRect() const;
        // <---
        void setData( const adcontrols::Trace& );
        void setData( const adcontrols::Chromatogram& );

        void setData( size_t, const double* x, const double* y );

        inline void push_back( const QPointF& d ) { values_.push_back( d ); }

    protected:
        size_t start_pos_;
        std::pair< double, double > range_x_;
        std::pair< double, double > range_y_;
        std::vector< QPointF > values_;
    };

}

