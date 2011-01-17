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

#include "dataprochandler.h"
#include <adcontrols/centroidprocess.h>

#include <adcontrols/isotopecluster.h>
#include <adcontrols/isotopemethod.h>

#include <adcontrols/tableofelements.h>
#include <adcontrols/chemicalformula.h>

using namespace dataproc;

DataprocHandler::DataprocHandler()
{
}

bool
DataprocHandler::doCentroid( adcontrols::MassSpectrum& res, const adcontrols::MassSpectrum& profile, const adcontrols::CentroidMethod& m )
{
    adcontrols::CentroidProcess peak_detector;
    if ( peak_detector( m, profile ) )
        return peak_detector.getCentroidSpectrum( res );
    return false;
}

bool
DataprocHandler::doIsotope( adcontrols::MassSpectrum& res, const adcontrols::IsotopeMethod& m )
{
    adcontrols::ChemicalFormula chemicalFormula 
        = adcontrols::TableOfElements::instance()->getChemicalFormula();

    adcontrols::IsotopeCluster cluster;
    cluster.clearFormulae();

    for ( adcontrols::IsotopeMethod::vector_type::const_iterator it = m.begin(); it != m.end(); ++it ) {
        std::wstring stdFormula = chemicalFormula.standardFormula( it->formula );
        cluster.addFormula( stdFormula, it->adduct, it->chargeState, it->relativeAmounts );
    }
    size_t nPeaks(0);
    if ( cluster.computeFormulae( m.threshold(), true, m.resolution(), res, nPeaks, m.useElectronMass() ) ) {
        return true;
    }
    return false;
}
