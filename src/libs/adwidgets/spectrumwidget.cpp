//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "spectrumwidget.h"
#include "axis.h"

using namespace adwidgets::ui;

SpectrumWidget::~SpectrumWidget()
{
}

SpectrumWidget::SpectrumWidget(QWidget *parent) :
    DataplotWidget(parent)
{
    this->axisX().text( L"m/z" );
    this->axisY().text( L"Intensity" );
}
