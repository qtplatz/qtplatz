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

#pragma once

#include "annotation.hpp"
#include <vector>
#include <string>
#include <qnamespace.h>
#include <qwt_plot.h>
class QwtText;
class QPointF;
class QRectF;
class QSizeF;

namespace adplot {

    class Annotation;
    class plot;

    class Annotations {
    public:
        typedef std::vector< Annotation > vector_type;

        Annotations( plot&, vector_type& );
        Annotations( const Annotations& );

        Annotation& add( double x = 0.0, double y = 0.0, const std::wstring& title = L"", QwtPlot::Axis yAxis = QwtPlot::yLeft );
        bool insert( double x, double y, QwtPlot::Axis, const QwtText& label, Qt::Alignment align = Qt::AlignTop | Qt::AlignHCenter );
        bool insert( double x, double y, QwtPlot::Axis, QwtText&& label, Qt::Alignment align = Qt::AlignTop | Qt::AlignHCenter );

        void clear();
        inline size_t size() const { return vec_.size(); }
        inline vector_type::iterator begin() { return vec_.begin(); }
        inline vector_type::iterator end() { return vec_.end(); }
        inline vector_type::const_iterator begin() const { return vec_.begin(); }
        inline vector_type::const_iterator end() const { return vec_.end(); }

    private:
        vector_type& vec_;
        plot& plot_;
        bool interference( double x, double y, const QwtText&, Qt::Alignment ) const;
        QRectF boundingRect( const Annotation&, Qt::Alignment ) const;
        QRectF boundingRect( double x, double y, const QwtText& label, Qt::Alignment align ) const;
        void adjust( QRectF& rc, Qt::Alignment ) const;
    };

}
