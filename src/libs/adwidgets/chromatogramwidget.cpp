//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "chromatogramwidget.h"
#include <adwidgets/axis.h>

using namespace adwidgets::ui;

ChromatogramWidget::~ChromatogramWidget()
{
}

ChromatogramWidget::ChromatogramWidget(QWidget *parent) : TraceWidget(parent)
{
    this->axisX().text( L"Time(min)");
    this->axisY().text( L"(uV)");
    this->title( 0, L"Chromatogram" );
}
