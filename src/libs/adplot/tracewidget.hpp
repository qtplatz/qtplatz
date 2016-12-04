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

namespace adcontrols { class MassSpectrum; }

namespace adplot {

    namespace internal { class TraceData; }

    class ADPLOTSHARED_EXPORT TraceWidget : public plot {
        Q_OBJECT
    public:
        explicit TraceWidget(QWidget *parent = 0);
        ~TraceWidget();

        void clear();
        void setData( std::size_t, const double * px, const double * py, int idx = 0, bool axisRight = false );
        void xBottomTitle( const std::string& );
        void yLeftTitle( const std::string& );

    private:
        struct TraceWidgetImpl * impl_;
        bool autoYZoom_;

        QwtText tracker1( const QPointF& );
        QwtText tracker2( const QPointF&, const QPointF& );

    signals:
        void onMoved( const QPointF& );
		void onSelected( const QPointF& );
		void onSelected( const QRectF& );

	private:
        virtual void zoom( const QRectF& );

    public slots:
		virtual void override_zoom_rect( QRectF& );
		virtual void moved( const QPointF& );
		virtual void selected( const QPointF& );
		virtual void selected( const QRectF& );
    };

}

