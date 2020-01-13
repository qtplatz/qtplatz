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
#include <functional>
#include <memory>
#include <tuple>
class QwtPlotZoomer;

namespace adcontrols { class MassSpectrum; class MassSpectrometer; }

namespace adplot {

    class ADPLOTSHARED_EXPORT SpectrumWidget : public plot {
        Q_OBJECT
    public:
        explicit SpectrumWidget(QWidget *parent = 0);
        ~SpectrumWidget();

        void clear();
        void removeData( int idx, bool replot = true );
        void setData( std::shared_ptr< const adcontrols::MassSpectrum >, int idx, bool axisRight = false );
        void setAlpha( int idx, int alpha );
        void setColor( int idx, const QColor& color );
        void setFocusedFcn( int fcn );

        enum HorizontalAxis { HorizontalAxisMass, HorizontalAxisTime };
        void setAxis( HorizontalAxis
                      , bool replot = false
                      , std::function< QRectF( const QRectF&, const adcontrols::MassSpectrum&, HorizontalAxis ) > f
                      = std::function< QRectF( const QRectF&, const adcontrols::MassSpectrum&, HorizontalAxis ) >() );
        HorizontalAxis axis() const;
        bool autoAnnotation() const;
        void setAutoAnnotation( bool enable = true );
        void update_annotation( bool replot = true );
        void setKeepZoomed( bool );
        void setZoomBase( const std::pair< double, double >& range, bool horizontal = true );
        void setZoomStack( const QRectF& );
        void setVectorCompression( int ) override;
        void yZoom( double xmin, double xmax ) override;
        static QColor index_color( unsigned int idx );

        void rescaleY( int fcn );
        void setViewId( uint32_t id );
        uint32_t viewId() const;
        void setYScale( double top, double bottom, bool axisRight = false );
        std::tuple< bool, double, double > yScale( bool ) const;

    private:
        class impl;
        impl * impl_;
        uint32_t viewid_;
        void redraw_all( bool keepX = false );
        void yScaleHock( QRectF& );


    signals:
        void onMoved( const QPointF& );
		void onSelected( const QRectF& );

    public slots:
		virtual void moved( const QPointF& );
		virtual void selected( const QPointF& );
		virtual void selected( const QRectF& );
		void zoomed( const QRectF& );
    };

}
