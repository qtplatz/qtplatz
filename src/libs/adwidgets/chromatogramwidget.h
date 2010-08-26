// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef CHROMATOGRAMWIDGET_H
#define CHROMATOGRAMWIDGET_H

#include "tracewidget.h"

namespace adwidgets {
    namespace ui {

        class ChromatogramWidget : public TraceWidget {
            Q_OBJECT
        public:
            ~ChromatogramWidget();
            explicit ChromatogramWidget(QWidget *parent = 0);

        signals:

        public slots:

        };
    }
}

#endif // CHROMATOGRAMWIDGET_H
