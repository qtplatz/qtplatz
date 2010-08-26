//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "tracewidget.h"
#include <adwidgets/axis.h>
#include <adwidgets/titles.h>
#include <adwidgets/title.h>

using namespace adwidgets::ui;

TraceWidget::~TraceWidget()
{
}

TraceWidget::TraceWidget(QWidget *parent) :
    DataplotWidget(parent)
{
}

void
TraceWidget::title( int index, const std::wstring& text )
{
    if ( index < 0 || unsigned(index) > titles().count() )
		return;
    Title title = titles()[ index ];
    title.text( text.c_str() );
    title.visible( true );
	titles().visible( true );
}