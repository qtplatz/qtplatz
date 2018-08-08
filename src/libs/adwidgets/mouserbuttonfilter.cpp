/**************************************************************************
** Copyright (C) 2010-     Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2018 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com or info@ms-cheminfo.com
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

#include "mouserbuttonfilter.hpp"
#include <QEvent>
#include <QMouseEvent>
#include <QWidget>

using namespace adwidgets;

MouseRButtonFilter::MouseRButtonFilter( QObject * parent ) : QObject( parent )
{
}

MouseRButtonFilter::~MouseRButtonFilter()
{
}


bool
MouseRButtonFilter::eventFilter( QObject * obj, QEvent * ev )
{
    if ( (ev->type() == QEvent::MouseButtonRelease ) && obj->isWidgetType() )
        if ( static_cast< QMouseEvent * >(ev)->button() == Qt::RightButton )
            emit mouseEvent( qobject_cast< QWidget * >( obj ), static_cast< QMouseEvent * >( ev ) );
    return false;
}

void
MouseRButtonFilter::installOn( QWidget * w )
{
    w->installEventFilter( this );
}

