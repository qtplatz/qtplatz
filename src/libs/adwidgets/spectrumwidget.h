// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#include "tracewidget.h"

namespace adcontrols {
    class MassSpectrum;
}

namespace adwidgets {
    namespace ui {

        class SpectrumWidget : public TraceWidget {
            Q_OBJECT
        public:
			~SpectrumWidget();
            explicit SpectrumWidget(QWidget *parent = 0);

            void setData( const adcontrols::MassSpectrum& );
			void setData( const adcontrols::MassSpectrum&, const adcontrols::MassSpectrum& );
			void setData( const adcontrols::MassSpectrum&, int idx, bool yaxis1 = false );

        signals:

        public slots:
            virtual void handleZoomXY( double x1, double y1, double x2, double y2 );
            virtual void handleZoomXAutoscaleY( double x1, double x2 );
        };

    }
}


