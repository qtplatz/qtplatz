/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
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

#include "tofprocessed.hpp"

using namespace tofinterface;

SpectrumPeakInfo::SpectrumPeakInfo() : monitor_range_lower(0)
                                     , monitor_range_upper(0)
                                     , peakMass(0)
                                     , peakHeight(0)
                                     , peakArea(0)
                                     , resolvingPower(0)
                                     , resolvingPowerX1(0)
                                     , resolvingPowerX2(0)
                                     , resolvingPowerHH(0)
{
}

tofProcessedData::tofProcessedData() : tic(0)
                                     , spectralBaselineLevel(0)
                                     , uptime(0)
{
}

tofProcessedData::tofProcessedData( const tofProcessedData& t ) : tic( t.tic )
                                                                , spectralBaselineLevel( t.spectralBaselineLevel )
                                                                , uptime( t.uptime )
                                                                , info( t.info )
{
}

