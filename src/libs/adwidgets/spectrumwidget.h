// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef SPECTRUMWIDGET_H
#define SPECTRUMWIDGET_H

// #include <adwidgets/dataplotwidget.h>
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

		private:
			void setData( const adcontrols::MassSpectrum&, int idx, bool yaxis1 = false );

        signals:

        public slots:
            virtual void handleZoomXY( double x1, double y1, double x2, double y2 );
            virtual void handleZoomXAutoscaleY( double x1, double x2 );
        };

    }
}

#endif // SPECTRUMWIDGET_H
