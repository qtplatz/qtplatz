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
#include <adportable/combination.hpp>
#ifdef _DEBUG
#include <iostream>
#endif

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

namespace adcontrols {
	struct atom {
		size_t idx;
		const Element * element;
		atom( const Element& e, size_t id ) : idx( id ), element( &e ){
		}
		atom( const atom& t ) : idx( t.idx ), element( t.element ) {
		}
		void operator = ( const atom& t ) {
			idx = t.idx;
			element = t.element;
		} 
	};
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

		std::vector< atom > vec;

		const Element& e = toe->findElement( it->first );
		for ( size_t i = 0; i < e.isotopeCount(); ++i ) 
			vec.push_back( atom( e, i ) );

		size_t natom = it->second;
		std::vector< std::vector< atom >::const_iterator > v( natom, vec.begin() );

		do {
#ifdef _DEBUG
			for ( int i = 0; i < natom; ++i ) {
				std::wcout << i << v[i]->element->symbol() << ", ";
			}
#endif
		} while ( boost::next_mapping( v.begin(), v.end(), vec.begin(), vec.end() ) );

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
