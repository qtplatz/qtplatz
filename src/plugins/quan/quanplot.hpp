/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#ifndef QUANPLOT_HPP
#define QUANPLOT_HPP

#include <QObject>
#include <memory>
#include <vector>
#if !defined Q_MOC_RUN
#include "quanpublisher.hpp"
#endif

namespace adwplot { class Dataplot; }

class QwtPlotCurve;
class QwtPlotMarker;

namespace quan {

    class QuanPlot : QObject {
        Q_OBJECT
        QuanPlot( const QuanPlot& ) = delete;
    public:
        QuanPlot();
        QuanPlot( std::vector< std::shared_ptr< QwtPlotCurve > >&
                  , std::vector< std::shared_ptr< QwtPlotMarker > >& );

        void plot_response_marker_yx( adwplot::Dataplot* plot, double intensity, double amount, const std::pair<double,double>& );
        void plot_calib_curve_yx( adwplot::Dataplot* plot, const QuanPublisher::calib_curve& calib );
        
    private:
        std::vector< std::shared_ptr< QwtPlotCurve > > curves_holder_;
        std::vector< std::shared_ptr< QwtPlotMarker > > markers_holder_;

        std::vector< std::shared_ptr< QwtPlotCurve > >& curves_;
        std::vector< std::shared_ptr< QwtPlotMarker > >& markers_;
    };
}

#endif // QUANPLOT_HPP
