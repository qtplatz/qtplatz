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
