// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#include "chemicalformula.hpp"
#include "element.hpp"
#include "isotopecluster.hpp"
#include "isotopes.hpp"
#include "massspectrum.hpp"
#include "molecule.hpp"
#include "tableofelement.hpp"
#include <adportable/debug.hpp>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>
#include <numeric>
#include <sstream>

using namespace adcontrols;

isotopeCluster::isotopeCluster() : threshold_daltons_( 1.0e-8 )
                                 , threshold_abundance_( 1.0e-12 ) 
{
}

double
isotopeCluster::threshold_daltons() const
{
    return threshold_daltons_;
}

void
isotopeCluster::threshold_daltons( double d )
{
    threshold_daltons_ = d;
}

bool
isotopeCluster::operator()( mol::molecule& mol, int charge ) const
{
    mol.cluster.clear();
    mol << mol::isotope( 0.0, 1.0 ); // trigger calculation

    // loop for each element e.g. 'C', 'H', 'N', ...
    for ( auto& element : mol.elements ) {

        // loop for element count e.g. C6 
        for ( int k = 0; k < element.count(); ++k ) {
            
            std::vector< mol::isotope > cluster;
            
            for ( auto& p: mol.cluster ) {
                
                for ( auto& i: element.isotopes() ) {
                    
                    mol::isotope mi( p.mass + i.mass, p.abundance * i.abundance );
                    
                    // make an array of order of mass
                    auto it = std::lower_bound( cluster.begin(), cluster.end(), mi.mass
                                                , []( const mol::isotope& mi, const double& mass ){
                                                    return mi.mass < mass;
                                                });
                    if ( it == cluster.end() || !merge( *it, mi ) )
                        cluster.emplace( it, mi );
                }
            }
            
            mol.cluster = std::move( cluster );
        }
    }

    if ( charge > 0 ) {
        std::for_each( mol.cluster.begin(), mol.cluster.end()
                       , [&]( mol::isotope& pk ){ pk.mass = ( pk.mass - TableOfElement::instance()->electronMass() * charge ) / charge;});
    } else if ( charge < 0 ) {
        std::for_each( mol.cluster.begin(), mol.cluster.end(), [&]( mol::isotope& pk ){ pk.mass /= std::abs( charge ); });
    }
    return true;
}

bool
isotopeCluster::merge( mol::isotope& it, const mol::isotope& mi ) const
{
    if ( mi.abundance <= threshold_abundance_ )
        return true; // throw it away

    if ( std::abs( it.mass - mi.mass ) < threshold_daltons_ ) {
        it.abundance += mi.abundance;
        
        // weighting average for mass -- this may affected when other independent molecule is co-exist
        double m = ( it.mass * it.abundance + mi.mass * mi.abundance ) / ( it.abundance + mi.abundance );
        // assert( std::abs( it.mass - m ) < ( 2.0e-7 );
        it.mass = m;
        return true;
    }
    return false;
}

void
isotopeCluster::merge_peaks( std::vector< isopeak >& peaks, double resolving_power )
{
    std::vector< isopeak > merged;

    while ( !peaks.empty() ) {
        
        auto bp = std::max_element( peaks.begin(), peaks.end()
                                    , []( const isopeak& a, const isopeak& b ){ return a.abundance < b.abundance; } );
        const double width = bp->mass / resolving_power;

        auto lIt = std::lower_bound( peaks.begin(), peaks.end(), bp->mass - width / 2
                                    , []( const isopeak& a, double m ){ return a.mass < m; } );
        if ( lIt != peaks.end() ) {
            auto uIt = std::lower_bound( peaks.begin(), peaks.end(), bp->mass + width / 2
                                         , []( const isopeak& a, double m ){ return a.mass < m; } );

            isopeak pk = std::accumulate( lIt, uIt, isopeak(), []( const isopeak& a, const isopeak& b ){
                    return isopeak( a.mass + ( b.mass * b.abundance ), a.abundance + b.abundance );
                });
            pk.mass /= pk.abundance;

            auto it = std::lower_bound( merged.begin(), merged.end(), pk.mass
                                        , []( const isopeak& a, double m ){ return a.mass < m; } );
            merged.emplace( it, pk );

            peaks.erase( lIt, uIt );
        }
    }

    peaks = merged;
}

bool
isotopeCluster::operator()( std::vector< isopeak >& mi
                            , const std::string& formula, double relative_abundance, int idx ) const
{
    if ( formula.empty() || relative_abundance <= 0.0 )
        return false;

    int charge(0);
    mol::molecule mol;
    ChemicalFormula::getComposition( mol.elements, formula, charge );

    (*this)( mol, charge );

    auto maxIt = std::max_element( mol.cluster.begin(), mol.cluster.end()
                                   , [] ( const mol::isotope& a, const mol::isotope& b ) { return a.abundance < b.abundance; } );
    double pmax = maxIt->abundance;

    auto tail = mol.cluster.end();

    if ( mol.elements.size() > 1 )  {
        tail = std::remove_if( mol.cluster.begin(), mol.cluster.end()
                               , [pmax]( const mol::isotope& i ) { return i.abundance / pmax < 1.0e-12; } );
    }

    std::for_each( mol.cluster.begin(), tail, [&]( const mol::isotope& i ){
            auto it = std::lower_bound( mi.begin(), mi.end(), i.mass, [idx] ( const isopeak& a, double m ) { return a.mass < m; } );
            mi.insert( it, isopeak( i.mass, i.abundance, idx ) );
        });
    
    return true;
}

bool
isotopeCluster::operator()( MassSpectrum& ms, const std::string& formula
                            , double relative_abundance, double resolving_power ) const
{
    std::vector< std::pair< std::string, double > > f;

    f.emplace_back( formula, relative_abundance );
    
    return ( *this )( ms, f, resolving_power );
}

bool
isotopeCluster::operator()( MassSpectrum& ms
                            , const std::vector< std::pair<std::string, double > >& formula_abundances
                            , double resolving_power ) const
{
    std::vector< isopeak > peaks;
    
    for ( auto& formula_abundance: formula_abundances )
        ( *this )( peaks, formula_abundance.first, formula_abundance.second );
    
    if ( ! peaks.empty() ) {
        
        merge_peaks( peaks, resolving_power );
        ms.resize( peaks.size() );
        
        size_t idx(0);
        
        for ( auto& i: peaks ) {
            ms.setMass( idx, i.mass );
            ms.setIntensity( idx, i.abundance * 100 );
            ++idx;
        }

        return true;
    }
    return false;
}

namespace adcontrols {
    namespace molformula {

        struct isotope {
            double mass;
            double abundance;
            std::map< std::pair< std::string, int >, size_t > elist;

            isotope( double m = 0, double a = 1.0 ) : mass(m), abundance(a) {
            }

            isotope( const isotope& p
                     , const std::string& symbol
                     , const toe::isotope& i ) : mass( p.mass + i.mass )
                                               , abundance( p.abundance * i.abundance ) {
                elist[ std::make_pair( symbol, int( i.mass + 0.3 ) ) ]++;
            }            
        };
        
        ////////////////
        struct molecule {
            std::vector< isotope > cluster;
            
            molecule() {}
            
            molecule( const molecule& t ) : cluster( t.cluster ) {
            }
        };
    }
}

// static
std::vector< std::string >
isotopeCluster::formulae( const std::string& formula )
{
    return std::vector< std::string >();
    
    int charge(0);
    std::vector< mol::element > elements;
    ChemicalFormula::getComposition( elements, formula, charge );

    molformula::molecule mol;
    mol.cluster.emplace_back( 0, 1.0 );

    // loop for each element e.g. 'C', 'H', 'N', ...
    for ( auto& element : elements ) {

        // loop for element count e.g. C6 
        for ( int k = 0; k < element.count(); ++k ) {
            
            std::vector< molformula::isotope > cluster;
            
            for ( auto& p: mol.cluster ) {
                // ADDEBUG() << "mol.cluser.size: " << mol.cluster.size();
                for ( auto& i: element.isotopes() ) {
                    //molformula::isotope mi( p.mass + i.mass, p.abundance * i.abundance );
                    molformula::isotope mi( p, element.symbol(), i );
                    cluster.emplace_back( mi );
                }
            }
            // ADDEBUG() << "cluser.size: " << cluster.size();
            mol.cluster = std::move( cluster );
        }
    }

    std::vector< std::string > result;
    ADDEBUG() << "mol size: " << mol.cluster.size();
    
    for ( const auto& m: mol.cluster ) {
        std::ostringstream o;
        if ( charge )
            o << "[";
        for ( auto& a: m.elist )
            o << a.first.second << a.first.first << a.second << ' '; // 13C6_
        if ( charge ) {
            o << "]";
            if ( charge > 1 )
                o << std::abs( charge );
            o << ( charge > 0 ? "+" : "-" );
        }
        result.emplace_back( o.str() );
    }

    return result;
}
