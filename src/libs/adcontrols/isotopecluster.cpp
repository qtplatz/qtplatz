// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include <compiler/disable_unused_parameter.h>
#include "isotopecluster.hpp"
#include "tableofelements.hpp"
#include "massspectrum.hpp"
#include "description.hpp"
#include "chemicalformula.hpp"
#include "element.hpp"
#include <adportable/combination.hpp>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>
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
		static inline void init( iterator_t it, iterator_t end, size_t natoms ) {
			*it++ = natoms;
			while ( it != end )
				*it++ = 0;
		} 

		template<class iterator_t>
		static inline bool next( iterator_t it, iterator_t end, size_t natoms ) {
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

		static double combination( size_t n, size_t r ) {
			double a = n;
			if ( r > 0 ) {
				size_t x = n; 
				while ( --x > ( n - r ) )
					a *= x;
				do {
					a /= r;
				} while( --r );
				return a;
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

	struct cluster {
		std::wstring symbol;
		size_t natoms;
		size_t nrotate;
		bool status;
		std::vector< std::pair< double, double > > ma;
		cluster( const std::wstring& s, size_t n ) : symbol( s ), natoms( n ), nrotate( 0 ), status(false) {
		}
		void operator = ( const cluster& t ) {
			symbol = t.symbol;
			natoms = t.natoms;
			nrotate = t.nrotate;
			status = t.status;
			ma = t.ma;
		}
		bool rotate() {
			std::rotate( ma.begin(), ma.begin() + 1, ma.end() );
			status = false;
			if ( ++nrotate == ma.size() ) {
				nrotate = 0;
				status = true;
			}
			return status;
		}
		operator bool () const { 
			return status;
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

	adcontrols::TableOfElements *toe = adcontrols::TableOfElements::instance();
	ChemicalFormula::elemental_composition_map_t ecomp = ChemicalFormula::getComposition( formula );

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
			if ( a >= 1.0e-4 )
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
	for ( ;; ) {
		std::vector< cluster >::reverse_iterator atom = atoms.rbegin(); // atom != atoms.rend(); ++atom ) {

		for ( size_t n = 0; n < atom->ma.size(); ++ n ) {

			std::pair< double, double > ma( 0.0, 1.0 );
			for ( std::vector< cluster >::const_iterator it = atoms.begin(); it != atoms.end(); ++it ) {

				const std::pair< double, double >& isotope = it->ma[ 0 ];
#if defined _DEBUG && 0
				std::wcout << std::setw(3) << it->symbol << it->natoms;
				std::cout << " (" << int ( isotope.first + 0.2 ) << ")";
#endif
				ma.first += isotope.first;
				ma.second *= isotope.second;
			}
            if ( ma.second >= 1.0e-4 )
				distribution.push_back( ma );

#if defined _DEBUG && 0
			std::cout << "\t" << int( ma.first + 0.2 ) << std::fixed << std::setprecision(3) << " (" << ma.second << ")" << std::endl;
#endif
			std::vector< cluster >::reverse_iterator prev = atom;
			while ( prev->rotate() && ( atoms.rend() != prev + 1 ) )
				prev++;
		}
#if defined _DEBUG && 0
		std::cout << std::endl;
#endif
		if ( *atoms.begin() )
			break;
	}
	std::sort( distribution.begin(), distribution.end()
		, boost::bind( &std::pair<double, double>::first, _1 ) < boost::bind( &std::pair<double, double>::first, _2 ) );

	std::pair<double, double> pos = *std::max_element( distribution.begin(), distribution.end()
		, boost::bind( &std::pair<double,double>::second, _1 ) < boost::bind( &std::pair<double, double>::second, _2 ) );

	ms.addDescription( adcontrols::Description( L"isocluster", formula ) );
	ms.setCentroid( adcontrols::CentroidIsotopeSimulation );
    ms.resize( distribution.size() );
    
    size_t idx = 0;
	for ( std::vector< std::pair<double, double> >::iterator it = distribution.begin(); it != distribution.end(); ++it, ++idx ) {
		it->second /= pos.second;
		ms.setMass( idx, it->first );
		ms.setIntensity( idx, it->second * 1000 );
	}

#if defined _DEBUG
	for ( std::vector< std::pair<double, double> >::iterator it = distribution.begin(); it != distribution.end(); ++it )
		std::cout << it->first << ", " << it->second << std::endl;
#endif

	return true;
}

//////////////////////

IsotopeClusterImpl::~IsotopeClusterImpl()
{
}

IsotopeClusterImpl::IsotopeClusterImpl()
{
}
