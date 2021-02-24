// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2021 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2021 MS-Cheminformatics LLC
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

#include "annotation.hpp"
#include "annotations.hpp"
#include "chemicalformula.hpp"
#include "element.hpp"
#include "isotopecluster.hpp"
#include "isotopes.hpp"
#include "lapfinder.hpp"
#include "massspectrometer.hpp"
#include "massspectrum.hpp"
#include "molecule.hpp"
#include "scanlaw.hpp"
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
                                 , resolving_power_( 100000 )
{
}

isotopeCluster::isotopeCluster( double abundance_threshold, double resolving_power ) : threshold_daltons_( 1.0e-8 )
                                                                                     , threshold_abundance_( abundance_threshold )
                                                                                     , resolving_power_( resolving_power )
{
}

double
isotopeCluster::threshold_daltons() const
{
    return threshold_daltons_;
}

void
isotopeCluster::setThreshold_daltons( double d )
{
    threshold_daltons_ = d;
}

bool
isotopeCluster::compute( mol::molecule& mol, int charge ) const
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
        std::for_each( mol.cluster.begin(), mol.cluster.end()
                       , [&]( mol::isotope& pk ){ pk.mass = ( pk.mass - TableOfElement::instance()->electronMass() * (-charge) ) / (-charge);});
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

            isopeak pk = std::accumulate( lIt, uIt, isopeak()
                                          , []( const isopeak& a, const isopeak& b ){
                                                int index = a.abundance > b.abundance ? a.index : b.index;
                                                return isopeak( a.mass + ( b.mass * b.abundance ), a.abundance + b.abundance, index );
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

// targeting support method
std::vector< isotopeCluster::isopeak >
isotopeCluster::operator()( const std::vector< std::pair< std::string, char > >& formulae, int charge, int index )
{
    if ( formulae.empty() )
        return std::vector< isotopeCluster::isopeak >();

    double mass;
    std::tie( mass, charge ) = ChemicalFormula().getMonoIsotopicMass( formulae, charge ); // charge will be ignored if it is zero
    threshold_daltons_ = mass / resolving_power_ / 2;

    // ADDEBUG() << "molFormula : " << ChemicalFormula::make_formula_string( formulae )
    //           << ", adducts: " << ChemicalFormula::make_adduct_string( formulae )
    //           << ", mol: " << ChemicalFormula::standardFormula( formulae )
    //           << ", mass: " << mass
    //           << ", charge: " << charge;

    int ignore;
    mol::molecule mol;
    std::string stdformula;
    std::tie( stdformula, std::ignore ) = ChemicalFormula::standardFormula( formulae );

    ChemicalFormula::getComposition( mol.elements, stdformula, ignore );

    compute( mol, charge );

    std::vector< isotopeCluster::isopeak > pks;
    pks.reserve( mol.cluster.size() );

    const auto bp = std::max_element( mol.cluster.begin(), mol.cluster.end()
                                      , [] ( const mol::isotope& a, const mol::isotope& b ) { return a.abundance < b.abundance; } );

    std::for_each( mol.cluster.begin(), mol.cluster.end()
                   , [&]( const mol::isotope& i ){
                         double abundance = i.abundance / bp->abundance;
                         if ( abundance > threshold_abundance_ )
                             pks.emplace_back( i.mass, abundance, index );
                     });

    merge_peaks( pks, resolving_power_ );

    return pks;
}

bool
isotopeCluster::operator()( adcontrols::MassSpectrum& ms
                            , const std::vector< std::tuple< std::string, double, int > >& formula_mass_charge
                            , double resolving_power )
{
    // ADDEBUG() << "isotopeCluster rp:" << resolving_power << ", " << resolving_power_;
    resolving_power_ = resolving_power;

    std::vector< isotopeCluster::isopeak > peaks;
    int index(0);
    for ( const auto& formula: formula_mass_charge ) {
        auto pks = (*this)( ChemicalFormula::split( std::get<0>( formula ) ), std::get< 2 >( formula ), index++ );
        for ( const auto& pk: pks ) {
            auto it = std::lower_bound( peaks.begin(), peaks.end(), pk.mass, []( const auto& a, double m ){ return a.mass < m; });
            peaks.emplace( it, pk );
        }
    }


    if ( peaks.empty() )
        return false;

    merge_peaks( peaks, resolving_power );
    ms.resize( peaks.size() );

    size_t idx(0);
    auto& annots = ms.get_annotations();
    for ( auto& i: peaks ) {
        ms.setMass( idx, i.mass );
        ms.setIntensity( idx, i.abundance * 100 );
        ms.setColor( idx, static_cast<unsigned>(i.index) % 17 );
        //
        if ( formula_mass_charge.size() > i.index ) {
            std::string formula; double mass; int charge;
            std::tie( formula, mass, charge ) = formula_mass_charge[ i.index ];
            annots <<
                adcontrols::annotation( formula, mass, (i.abundance * 100 ), int( idx ), 0, adcontrols::annotation::dataFormula );
        }
        ++idx;
    }

    return true;
}

///////////////////////////////////////////
///// for multi-turn apparent m/z
bool
isotopeCluster::operator()( adcontrols::MassSpectrum& ms
                            , const std::vector< std::tuple< std::string, double, int > >& formula_mass_charge
                            , double resolving_power
                            , std::shared_ptr< const adcontrols::MassSpectrometer > sp
                            , int lap )
{
    resolving_power_ = resolving_power;

    std::vector< isotopeCluster::isopeak > peaks;
    int index(0);
    for ( const auto& fmc: formula_mass_charge ) {
#if __cplusplus >= 201703L
        auto [ formula, mass, charge ] = fmc;
#else
        std::string formula; double mass; int charge;
        std::tie( formula, mass, charge ) = fmc;
#endif
        // ADDEBUG() << "----- " << formula << ", " << mass << ", " << charge;
        //----------- develop a list of clusters ---------->
        auto pks = (*this)( ChemicalFormula::split( formula ), charge, index++ );
        //<------------------------------------------------
        for ( const auto& pk: pks ) {
            auto it = std::lower_bound( peaks.begin(), peaks.end(), pk.mass, []( const auto& a, double m ){ return a.mass < m; });
            peaks.emplace( it, pk );
        }
    }
    if ( peaks.empty() )
        return false;

    double base_mass(0);
    auto it = std::max_element( formula_mass_charge.begin(), formula_mass_charge.end()
                                , []( const auto a, const auto b ){ return std::get<1>(a) < std::get<1>(b); });
    if ( it != formula_mass_charge.end() )
        base_mass = std::get< 1 >(*it);

    for ( const auto& pk: peaks )
        ADDEBUG() << "----- : " << pk.mass;

    ADDEBUG() << "----- base mass: " << base_mass;

    merge_peaks( peaks, resolving_power );
    ms.resize( peaks.size() );

    if ( sp ) {
        if ( auto scanlaw = sp->scanLaw() ) {
            lapFinder finder( *scanlaw, base_mass, lap ); // <-- nlap
            size_t idx(0);
            auto& annots = ms.get_annotations();
            for ( const auto& pk: peaks ) {
                auto lt = finder( pk.mass );
                double aparent_mass = scanlaw->getMass( lt.second, lap ); // apparent mass
                ADDEBUG() << "----- : " << pk.mass << ", " << lt.first << ", " << lt.second << ", " << aparent_mass;
                ms.setMass( idx, aparent_mass ); // apparent mass
                ms.setTime( idx, lt.second );
                ms.setIntensity( idx, pk.abundance * 100 );
                if ( formula_mass_charge.size() > pk.index ) {
                    std::string formula; double mass; int charge;
                    std::tie( formula, mass, charge ) = formula_mass_charge[ pk.index ];
                    annots <<
                        adcontrols::annotation( formula, mass, (pk.abundance * 100 ), int( idx ), 0, adcontrols::annotation::dataFormula );
                }
                ++idx;
            }
        }
    } else {
        size_t idx(0);
        auto& annots = ms.get_annotations();
        for ( auto& i: peaks ) {
            ms.setMass( idx, i.mass );
            ms.setIntensity( idx, i.abundance * 100 );
            ms.setColor( idx, static_cast<unsigned>(i.index) % 17 );
            //
            if ( formula_mass_charge.size() > i.index ) {
                std::string formula; double mass; int charge;
                std::tie( formula, mass, charge ) = formula_mass_charge[ i.index ];
                annots <<
                    adcontrols::annotation( formula, mass, (i.abundance * 100 ), int( idx ), 0, adcontrols::annotation::dataFormula );
            }
            ++idx;
        }
    }
    return true;
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
