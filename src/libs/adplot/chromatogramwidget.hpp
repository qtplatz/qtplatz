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

#include "plot.hpp"
#include "adplot_global.hpp"
#include "annotation.hpp"
#include "baseline.hpp"
#include "peak.hpp"
#include <functional>

namespace adcontrols {
    class Trace;
    class Chromatogram;
	class PeakResult;
    class Baseline;
    class Peak;
    class annotations;
}

namespace adplot {

    class Peak;
    class Baseline;
    class SeriesData;
    class Annotation;

    class ADPLOTSHARED_EXPORT ChromatogramWidget : public plot {
        Q_OBJECT
    public:
        explicit ChromatogramWidget(QWidget *parent = 0);
		~ChromatogramWidget();

        [[deprecated]] void setData( std::shared_ptr< const adcontrols::Trace>, int idx = 0, bool yaxis2 = false );
        [[deprecated]] void setData( std::shared_ptr< const adcontrols::Chromatogram >, int idx = 0, bool axisRight = false );
        void setTrace( std::shared_ptr< const adcontrols::Trace>, int idx, QwtPlot::Axis );
        void setData( std::shared_ptr< const adcontrols::Chromatogram >, int idx, QwtPlot::Axis );
        std::shared_ptr< const adcontrols::Chromatogram > getData( int idx ) const;

    public:
        // void setData( std::shared_ptr< const adcontrols::Chromatogram >&&, int idx = 0, bool axisRight = false );
		void setData( const adcontrols::PeakResult& );
        void setAlpha( int idx, int alpha );
        void setColor( int idx, const QColor& color );
        void setNormalizedY( QwtPlot::Axis, bool );
        void clear();
        void removeData( int idx, bool report = true );
        void register_tracker( std::function< bool( const QPointF&, QwtText& ) > );
        void drawPeakParameter( const adcontrols::Peak& );
        void setZoomed( const QRectF&, bool keepY = true );
        QColor color( int idx ) const;
        QwtPlotItem * getPlotItem( int idx );

        enum HorizontalAxis { HorizontalAxisSeconds, HorizontalAxisMinutes };
        void setAxis( HorizontalAxis, bool replot = false );
        void setItemLegendEnabled( bool );
        bool itemLegendEnabled() const;
        void setLegendEnabled( bool );
        bool legendEnabled() const;

	private:
        void setBaseline( const adcontrols::Baseline& );
        void setPeak( const adcontrols::Peak&, adcontrols::annotations& );

    signals:
		void onMoved( const QPointF& );
		void onSelected( const QPointF& );
		void onSelected( const QRectF& );

    public slots:
		virtual void moved( const QPointF& );
		virtual void selected( const QPointF& );
		virtual void selected( const QRectF& );

    private slots:
		void zoomed( const QRectF& );

    private:
        void plotAnnotations( const adcontrols::annotations& );
        class impl;
        impl * impl_;
    };

}
