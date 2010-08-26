//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "spectrumwidget.h"
#include <adwidgets/axis.h>

using namespace adwidgets::ui;

SpectrumWidget::~SpectrumWidget()
{
}

SpectrumWidget::SpectrumWidget(QWidget *parent) :  TraceWidget(parent)
{
    this->axisX().text( L"m/z" );
    this->axisY().text( L"Intensity" );
	this->title( 0, L"Spectrum" );
}
