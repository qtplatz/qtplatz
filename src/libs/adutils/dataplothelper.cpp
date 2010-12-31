/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

#include "dataplothelper.h"
#include <adcontrols/timeutil.h>
#include <adcontrols/peak.h>
#include <adcontrols/baseline.h>
#include <adwidgets/adwidgets.h>
#include <adwidgets/peak.h>
#include <adwidgets/Baseline.h>

using namespace adutils;

DataplotHelper::DataplotHelper()
{
}

void
DataplotHelper::copy( adwidgets::ui::Peak& dst, const adcontrols::Peak& src )
{
    dst.startX( adcontrols::timeutil::toMinutes( src.startTime() ) );
    dst.startY( src.startHeight() );
    dst.centreX( adcontrols::timeutil::toMinutes( src.peakTime() ) );
    dst.centreY( src.topHeight() );
    dst.endX( adcontrols::timeutil::toMinutes( src.endTime() ) );
    dst.endY( src.endHeight() );
    dst.startMarkerStyel( adwidgets::ui::PM_DownTriangle );
    dst.centreMarkerStyle( adwidgets::ui::PM_UpStick );
    dst.endMarkerStyle( adwidgets::ui::PM_DownTriangle );
    dst.marked( true );
    dst.visible( true );
}

void
DataplotHelper::copy( adwidgets::ui::Baseline& dst, const adcontrols::Baseline& src )
{
    dst.startX( adcontrols::timeutil::toMinutes( src.startTime() ) );
    dst.startY( src.startHeight() );
    dst.endX( adcontrols::timeutil::toMinutes( src.stopTime() ) );
    dst.endY( src.stopHeight() );
    dst.marked( true );
    dst.visible( true );
}
