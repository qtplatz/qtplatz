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

#include "isotopecluster.hpp"
#include "tableofelements.hpp"
#include "massspectrum.hpp"
#include "chemicalformula.hpp"
#include "element.hpp"

using namespace adcontrols;

namespace adcontrols {

    class IsotopeClusterImpl {
    public:
        ~IsotopeClusterImpl();
        IsotopeClusterImpl();
    };
}

IsotopeCluster::~IsotopeCluster()
{
    delete impl_;
}

IsotopeCluster::IsotopeCluster() : impl_( new IsotopeClusterImpl )
{
}

bool
IsotopeCluster::Compute( const std::wstring& formula, double threshold, bool resInDa, double rp, MassSpectrum& ms, size_t& nPeaks )
{
    (void)formula;
	(void)threshold;
	(void)resInDa;
	(void)rp;
	(void)ms;
	(void)nPeaks;
    return false;
}

bool
IsotopeCluster::Compute( const std::wstring& formula, double threshold, bool resInDa, double rp, MassSpectrum& ms, const std::wstring& adduct, size_t charges, size_t& nPeaks, bool bAccountForElectrons )
{
    (void)formula;
	(void)threshold;
	(void)resInDa;
	(void)rp;
	(void)ms;
	(void)nPeaks;
	(void)bAccountForElectrons;
	(void)charges;
	(void)adduct;
	(void)bAccountForElectrons;
    return false;
}

void
IsotopeCluster::clearFormulae()
{
}

bool
IsotopeCluster::addFormula( const std::wstring& formula, const std::wstring& adduct, size_t chargeState, double relativeAmount )
{
	(void)formula;
	(void)adduct;
	(void)chargeState;
	(void)relativeAmount;
    return false;
}

bool
IsotopeCluster::computeFormulae(double threshold, bool resInDa, double rp,	MassSpectrum& ms, size_t& nPeaks, bool bAccountForElectrons, double ra )
{
	(void)threshold;
	(void)resInDa;
	(void)rp;
	(void)ms;
	(void)nPeaks;
	(void)bAccountForElectrons;
	(void)ra;
    return false;
}

bool
IsotopeCluster::isotopeDistribution( adcontrols::MassSpectrum& ms
									, const std::wstring& formula
									, size_t charges, bool accountElectron ) 
{
    (void)charges;
    (void)accountElectron;
    (void)ms;
	adcontrols::TableOfElements *toe = adcontrols::TableOfElements::instance();
	ChemicalFormula::elemental_composition_map_t ecomp = ChemicalFormula::getComposition( formula );
    
	double mass = 0;
	for ( ChemicalFormula::elemental_composition_map_t::iterator it = ecomp.begin(); it != ecomp.end(); ++it  ) {
		const Element& e = toe->findElement( it->first );
		size_t nIsotopes = e.isotopeCount();
		for ( Element::vector_type::const_iterator iso = e.begin(); iso != e.end(); ++iso ) {
           
		}
        (void)nIsotopes;
	}
    (void)mass;
	return true;
}

//////////////////////

IsotopeClusterImpl::~IsotopeClusterImpl()
{
}

IsotopeClusterImpl::IsotopeClusterImpl()
{
}
