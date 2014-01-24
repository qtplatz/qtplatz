/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#include "picker.hpp"
#include <qwt_plot_canvas.h>
#include <qwt_picker_machine.h>
#include <QMouseEvent>

using namespace adwplot;

Picker::Picker( QWidget * parent ) : QwtPlotPicker( parent )
{
    setTrackerMode(QwtPicker::AlwaysOff);
    setMousePattern( QwtEventPattern::MouseSelect1,  Qt::RightButton );
    setStateMachine( new QwtPickerDragRectMachine() );

    setRubberBand( QwtPicker::RectRubberBand );
    setRubberBandPen( QColor(Qt::red) );
    setTrackerPen( QColor( Qt::blue ) );
}

void
Picker::widgetMousePressEvent( QMouseEvent * ev )
{
	QwtPicker::widgetMousePressEvent( ev );
}

void
Picker::widgetMouseReleaseEvent( QMouseEvent * ev )
{
	QwtPicker::widgetMouseReleaseEvent( ev );
}

void
Picker::widgetMouseDoubleClickEvent( QMouseEvent * ev )
{
	QwtPicker::widgetMouseDoubleClickEvent( ev );
}

void
Picker::widgetMouseMoveEvent( QMouseEvent * ev )
{
	QwtPicker::widgetMouseMoveEvent( ev );
}

bool
Picker::end( bool ok )
{
	return QwtPlotPicker::end( ok );
}
