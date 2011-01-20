// -*- C++ -*-
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

#include "samassspectrum.h"
#include "massspectrum.h"
#include "import_sacontrols.h"

using namespace adcontrols;
using namespace adcontrols::internal;

SAMassSpectrum::SAMassSpectrum()
{
}

void
SAMassSpectrum::copy( SACONTROLSLib::ISAMassSpectrum5* pi, const MassSpectrum& ms)
{
    pi->Count = ms.size();
    pi->IsCentroid = ms.isCentroid() ? VARIANT_TRUE : VARIANT_FALSE;
    pi->SetIntensityArrayDirect2( reinterpret_cast<IUnknown *>(const_cast<double *>(ms.getIntensityArray())) );
    pi->SetMassArrayDirect2( reinterpret_cast<IUnknown *>(const_cast<double *>(ms.getMassArray())) );
}

void
SAMassSpectrum::copy( MassSpectrum& ms, SACONTROLSLib::ISAMassSpectrum5* pi, double sf )
{
    ms.resize( pi->Count );
    ms.setCentroid( pi->IsCentroid ? CentroidPeakAreaWaitedMass : CentroidNone );
    for ( size_t i = 0; i < ms.size(); ++i ) {
        ms.setMass( i, pi->GetMass( i ) );
        ms.setIntensity( i, pi->GetIntensity( i ) * sf );
    }
}
