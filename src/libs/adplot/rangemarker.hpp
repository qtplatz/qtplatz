/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
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

#pragma once

#include "adplot_global.hpp"
#include <qwt_plot_item.h>
#include <memory>
#include <array>

class QwtPlotMarker;
class QwtPlot;

namespace adplot {

    class ADPLOTSHARED_EXPORT RangeMarker : public QwtPlotItem {

    public:
        enum fence { lower, upper };

        virtual ~RangeMarker();
        RangeMarker();
        RangeMarker( const QColor& );

        void setValue( fence, double value );
        void setValue( double lower_value, double upper_value );

        void setColor( const QColor& );
        const QColor& color() const;
 
    protected:
        virtual void draw( QPainter *painter, const QwtScaleMap &xMap, const QwtScaleMap &, const QRectF &canvasRect ) const override;
        
    private:
        std::pair< double, double > range_;
        QColor color_;
    };

}

