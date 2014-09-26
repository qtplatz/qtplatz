/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef PLOT_STDERROR_HPP
#define PLOT_STDERROR_HPP

#include "adplot_global.hpp"
#include <vector>
#include <memory>
#include <string>

class QwtPlotCurve;
class QwtPlotMarker;
class QwtPlot;
class QPointF;
template<class T> class QVector;

namespace adplot {

    class ADPLOTSHARED_EXPORT plot_stderror {
        plot_stderror( const plot_stderror& );
    public:
        plot_stderror();

        void title( const std::string& title );
        void operator()( const QVector< QPointF >& data, QwtPlot& );
        void clear();

    private:
        std::vector< std::shared_ptr< QwtPlotCurve > > curves_;
        std::vector< std::shared_ptr< QwtPlotMarker > > markers_;
        std::string title_;
    };

}

#endif // PLOT_STDERROR_HPP
