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

#ifndef CHROMATOGRAMWIDGET_H
#define CHROMATOGRAMWIDGET_H

#include "tracewidget.h"

namespace adcontrols {
    class Chromatogram;
    class Trace;
    class Peaks;
    class Baselines;
}

namespace adwidgets {

    class Trace;

    namespace ui {

        class ChromatogramWidget : public TraceWidget {
            Q_OBJECT
        public:
            ~ChromatogramWidget();
            explicit ChromatogramWidget(QWidget *parent = 0);

            void setData( const adcontrols::Trace&, int idx = 0, bool yaxis2 = false );
            void setData( const adcontrols::Chromatogram& );

        signals:

        public slots:


        private:
			void setData( const adcontrols::Chromatogram&, int idx, bool yaxis2 = false );
            void setPeaks( const adcontrols::Peaks&, const adcontrols::Baselines&, Trace& );
            void setAnnotations( const adcontrols::Peaks&, Trace& );
        };
    }
}

#endif // CHROMATOGRAMWIDGET_H
