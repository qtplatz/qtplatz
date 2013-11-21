// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#include "dataplot.hpp"

namespace adcontrols { class MassSpectrum; }

namespace adwplot {

    namespace internal { class TraceData; }

    class SpectrumWidget : public Dataplot {
        Q_OBJECT
    public:
        explicit SpectrumWidget(QWidget *parent = 0);
        ~SpectrumWidget();

        void clear();
        void setData( const std::shared_ptr< adcontrols::MassSpectrum >&, int idx, bool yaxis1 = false );

        enum HorizontalAxis { HorizontalAxisMass, HorizontalAxisTime };
        void setAxis( HorizontalAxis );
        bool autoAnnotation() const;
        void setAutoAnnotation( bool enable = true );

    private:
        struct SpectrumWidgetImpl * impl_;
        bool autoYZoom_;
        HorizontalAxis haxis_;

    signals:
        void onMoved( const QPointF& );
		void onSelected( const QPointF& );
		void onSelected( const QRectF& );
        
	private:
        // virtual void zoom( const QRectF& );

    public slots:
		virtual void override_zoom_rect( QRectF& );
		virtual void moved( const QPointF& );
		virtual void selected( const QPointF& );
		virtual void selected( const QRectF& );
		void zoomed( const QRectF& );
    };

}

