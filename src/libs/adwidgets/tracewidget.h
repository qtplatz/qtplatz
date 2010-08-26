// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <adwidgets/dataplotwidget.h>

namespace adwidgets {
    namespace ui {

        class TraceWidget : public DataplotWidget {
            Q_OBJECT
        public:
            ~TraceWidget();
            explicit TraceWidget(QWidget *parent = 0);

			void title( int index, const std::wstring& );

        signals:

        public slots:

        };
    }
}

