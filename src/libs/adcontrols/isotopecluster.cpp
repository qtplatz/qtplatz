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
#include <boost/foreach.hpp>
#include <cmath>
#include <numeric>
#ifdef _DEBUG
#include <iostream>
#include <iomanip>
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
		Element::vector_type::const_iterator iso;
		atom( size_t id, const Element& e ) : idx( id ), element( &e ), iso( e.begin() + id ) {
		}
		atom( const atom& t ) : idx( t.idx ), element( t.element ), iso( t.iso ) {
		}
	};

	struct combination {
		template<class iterator_t>
		static inline void init( iterator_t it, iterator_t& end, size_t natoms ) {
			*it++ = natoms;
			while ( it != end )
				*it++ = 0;
		} 

		template<class iterator_t>
		static inline bool next( iterator_t it, iterator_t& end, size_t natoms ) {
			if ( it == end )
				return false;
			size_t nfree = natoms - *it;
			if ( nfree == 0 || *(end - 1) == nfree ) {
				if ( ( *it ) == 0 )
					return false;
				(*it)--;     // decriment
				init( ++it, end, nfree + 1 );
				return true;
			}
			return next( boost::next( it ), end, nfree );
		}
	};

	struct partial_molecular_mass {

		static int combination( size_t n, size_t r ) {
			unsigned long long a = n;
			if ( r > 0 ) {
				size_t x = n; 
				while ( --x > ( n - r ) )
					a *= x;
				do {
					a /= r;
				} while( --r );
				return int(a);
			}
			return 0;
		}

		template<class iterator_t>
		static double abundance( const Element& e, iterator_t it, iterator_t end ) {

			size_t n = std::accumulate( it, end, 0 );
			if ( n == (*it) )
				return std::pow( e.begin()->abundance_, int( n ) );
			double a = 0;
			++it;
			for ( Element::vector_type::const_iterator iso = e.begin() + 1; it != end && iso != e.end(); ++it, ++iso ) {
				size_t nCr = combination( n, *it );
				a += std::pow( iso->abundance_, int( *it ) ) * nCr;
			}
			return a;
		}

		template<class iterator_t>
		static double calculate( const Element& e, iterator_t it, iterator_t end ) {
			double m = 0;
			for ( Element::vector_type::const_iterator iso = e.begin(); it != end && iso != e.end(); ++it, ++iso ) {
					m += (*it) * iso->mass_;
			}
			return m;
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

	struct cluster {
		std::wstring symbol;
		size_t natoms;
		std::vector< std::pair< double, double > > ma;
		cluster( const std::wstring& s, size_t n ) : symbol( s ), natoms( n ) {
		}
		void operator = ( const cluster& t ) {
			symbol = t.symbol;
			natoms = t.natoms;
			ma = t.ma;
		}
	};

	std::vector< cluster > atoms;

	for ( ChemicalFormula::elemental_composition_map_t::iterator it = ecomp.begin(); it != ecomp.end(); ++it  ) {

		const Element& element = toe->findElement( it->first );

		atoms.push_back( cluster( it->first /* symbol */, it->second /* natoms */) );
        cluster& cluster = atoms.back();

		std::vector< size_t > counts( element.isotopeCount(), 0 );

		combination::init( counts.begin(), counts.end(), it->second );
		do {
			double m = partial_molecular_mass::calculate( element, counts.begin(), counts.end() );
			double a = partial_molecular_mass::abundance( element, counts.begin(), counts.end() );
            if ( a > 1.0e-9 )
				cluster.ma.push_back( std::make_pair( m, a ) );
#if defined _DEBUG && 0
			std::wcout << std::setw(4) << element.symbol() << ": "
				<< std::setprecision( 6 ) << std::fixed << std::setw( 10 ) 
				<< m << " (" << std::setprecision(5) << a << ")\t";
			for ( size_t i = 0; i < element.isotopeCount(); ++i ) {
				std::cout << counts[i];
				if ( i + 1 < element.isotopeCount() )
					std::cout << ", ";
			}
			std::cout << std::endl;
#endif
		} while ( combination::next( counts.begin(), counts.end(), it->second ) );
	}

	// --- //
	std::vector< std::pair<double, double> > distribution;

	for ( std::vector< cluster >::iterator atom = atoms.begin(); atom != atoms.end(); ++atom ) {
		for ( size_t n = 0; n < atom->ma.size(); ++ n ) {
			for ( std::vector< cluster >::const_iterator it = atoms.begin(); it != atoms.end(); ++it ) {

				std::wcout << std::setw(4) << it->symbol << it->natoms;
				std::vector< std::pair< double, double > >::const_iterator isotope = it->ma.begin();
				std::cout << "\t" << std::setprecision( 6 ) << std::fixed << isotope->first 
					<< "(" << std::setprecision(4) << isotope->second << ")" << std::endl;
			}
			std::rotate( atom->ma.begin(), atom->ma.begin() + 1, atom->ma.end() );
		}
	}

	return true;
}

//////////////////////

IsotopeClusterImpl::~IsotopeClusterImpl()
{
}

IsotopeClusterImpl::IsotopeClusterImpl()
{
}
