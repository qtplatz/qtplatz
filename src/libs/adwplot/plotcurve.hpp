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

#include <qwt_plot_curve.h>
#include <memory>

template<class T> class QwtSeriesData;

namespace adwplot {

    class Dataplot;

    class PlotCurve {
    public:
        ~PlotCurve();
        PlotCurve( Dataplot& plot, const std::wstring& = L"" );
        PlotCurve( const PlotCurve& );
        PlotCurve& operator = ( const PlotCurve& );
        void setStyle( QwtPlotCurve::CurveStyle );
        void setData( const double * xData, const double * yData, size_t size );
		inline QwtPlotCurve * p() { return curve_.get(); }
    private:
		std::shared_ptr< QwtPlotCurve > curve_;
        QwtSeriesData<QPointF> * series_;
    };


}

