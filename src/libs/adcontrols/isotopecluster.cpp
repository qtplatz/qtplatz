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
#include "isocluster.hpp"
#include "isotopecluster.hpp"
#include "isotopes.hpp"
#include "lapfinder.hpp"
#include "massspectrometer.hpp"
#include "massspectrum.hpp"
#include "molecule.hpp"
#include "scanlaw.hpp"
#include "tableofelement.hpp"
#include <adportable/debug.hpp>
#include <adutils/constants.hpp> // clsid for massspectrometer
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

    std::string stdformula;
    std::tie( stdformula, std::ignore ) = ChemicalFormula::standardFormula( formulae );

    // ChemicalFormula::getComposition( mol.elements, stdformula, ignore );
    if ( auto mol = ChemicalFormula::toMolecule( stdformula ) ) {

        isoCluster( threshold_abundance_, resolving_power_ )( mol, mol.charge() );

        std::vector< isotopeCluster::isopeak > pks;
        pks.reserve( mol.cluster().size() );

        auto bp = mol.max_abundant_isotope();
        if ( bp != mol.cluster_end() ) {
            std::for_each( mol.cluster_begin(), mol.cluster_end()
                           , [&]( const mol::isotope& i ){
                               double abundance = i.abundance / bp->abundance;
                               if ( abundance > threshold_abundance_ )
                                   pks.emplace_back( i.mass, abundance, index );
                           });
            merge_peaks( pks, resolving_power_ );
        }
        return pks;
    }
    return {};
}

bool
isotopeCluster::operator()( adcontrols::MassSpectrum& ms
                            , const std::vector< std::tuple< std::string, double, int > >& formula_mass_charge
                            , double resolving_power )
{
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

// static
std::shared_ptr< adcontrols::MassSpectrum >
isotopeCluster::toMassSpectrum( const std::vector< adcontrols::mol::molecule >& molecules
                                , std::shared_ptr< const adcontrols::MassSpectrum > source
                                , std::shared_ptr< const adcontrols::MassSpectrometer > sp
                                , int lap )
{
    if ( molecules.empty() )
        return {};


    if ( sp && sp->massSpectrometerClsid() == qtplatz::infitof::iids::uuid_massspectrometer ) {
        return __toMTSpectrum( molecules, source, sp, lap );
    } else {
        return __toMassSpectrum( molecules, source, sp, lap );
    }
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


//static
std::shared_ptr< adcontrols::MassSpectrum >
isotopeCluster::__toMTSpectrum( const std::vector< adcontrols::mol::molecule >& molecules
                                , std::shared_ptr< const adcontrols::MassSpectrum > source
                                , std::shared_ptr< const adcontrols::MassSpectrometer > sp
                                , int assigned_lap )
{
    if ( !sp || ( sp && ! sp->scanLaw() ) )
        return {};

    auto size = std::accumulate( molecules.begin(), molecules.end(), size_t(0), []( size_t n, const auto& m ){ return m.cluster().size() + n; });

    // get 'base mass' := highest m/z
    auto hMol = std::max_element( molecules.begin(), molecules.end(), [](const auto& a, const auto& b){ return a.mass() < b.mass(); });
    double hMass = hMol->mass();

    // lap number finder
    lapFinder finder( *sp->scanLaw(), hMass, assigned_lap );

    std::vector< std::tuple< std::vector< mol::isotope >::const_iterator     // cluset
                             , std::vector< mol::molecule >::const_iterator  // molecule
                             , int                                           // own lap number
                             , double                                        // tof
                             , double                                        // aparent mass
                             >
                 > ipks;

    for ( auto mIt = molecules.begin(); mIt != molecules.end(); ++mIt ) {
        for ( auto it = mIt->cluster_begin(); it != mIt->cluster_end(); ++it ) {
            auto [ xlap, xtof ] = finder( it->mass );
            double apparent_mass = sp->scanLaw()->getMass( xtof, assigned_lap );
            ipks.emplace_back( it, mIt, xlap, xtof, apparent_mass );
        }
    }

    std::sort( ipks.begin(), ipks.end(), []( const auto& a, const auto& b ){ return std::get<3>(a) < std::get<3>(b); } );

    // for ( auto pk: ipks )
    //     ADDEBUG() << "find lap for " << std::get<0>(pk)->mass << " --> " << std::get<2>(pk) << ", " << std::get<3>(pk) << ", " << std::get<4>(pk);

    auto ms = std::make_shared< MassSpectrum >();
    if ( source )
        ms->clone( *source ); // shallow copy
    ms->resize( ipks.size() );
    ms->setCentroid( CentroidNative );

    size_t idx(0);
    for ( auto& a: ipks ) {
#if __cplusplus >= 201703L
        auto [ itIso, itMol, xlap, xtof, xmass ] =  a;
#else
        mol::molecule::const_cluster_iterator itIso; std::vector< mol::molecule >::const_iterator itMol; int xlap; double xtof, xmass;
        std::tie( itIso, itMol, xlap, xtof, xmass ) = a;
#endif
        auto idMol = std::distance( molecules.begin(), itMol );
        ms->setMass( idx, xmass );
        ms->setTime( idx, xtof );
        ms->setIntensity( idx, itIso->abundance * 100 );
        ms->setColor( idx, static_cast<unsigned>(idMol) % 17 );

        std::ostringstream o;
        if ( itMol->max_abundant_isotope() == itIso ) {
            o << ChemicalFormula::formatFormula( itMol->display_formula() )
              << " " << std::to_string( itIso->mass );
        } else {
            double delta = itIso->mass - itMol->max_abundant_isotope()->mass;
            int idelta = ( delta > 0 ) ? int( delta + 0.7 ) : int( delta - 0.7 );
            o << itMol->formula() << "(" << std::showpos << idelta << ")";
        }
        // ADDEBUG() << o.str() << "\t" << xmass << ", " << itIso->mass << ", " <<itMol->mass() << " lap: " << xlap << "/" << assigned_lap;

        ms->get_annotations()
            << annotation( o.str(), xmass, ms->intensity( idx ), int( idx ), 0, annotation::dataText );
        ++idx;
    }
    return ms;
}

//static
std::shared_ptr< adcontrols::MassSpectrum >
isotopeCluster::__toMassSpectrum( const std::vector< adcontrols::mol::molecule >& molecules
                                  , std::shared_ptr< const adcontrols::MassSpectrum > source
                  , std::shared_ptr< const adcontrols::MassSpectrometer > sp
                  , int mode )
{
    std::vector< std::pair< std::vector< mol::isotope >::const_iterator, std::vector< mol::molecule >::const_iterator > > ipks;
    for ( auto mIt = molecules.begin(); mIt != molecules.end(); ++mIt ) {
        for ( auto it = mIt->cluster_begin(); it != mIt->cluster_end(); ++it )
            ipks.emplace_back( it, mIt );
    }
    std::sort( ipks.begin(), ipks.end(), []( const auto& a, const auto& b ){ return a.first->mass < b.first->mass; } );

    auto ms = std::make_shared< MassSpectrum >();
    if ( source )
        ms->clone( *source ); // shallow copy
    ms->resize( ipks.size() );
    ms->setCentroid( CentroidNative );

    size_t idx(0);
    for ( auto& a: ipks ) {
#if __cplusplus >= 201703L
        auto [ itIso, itMol ] = a;
#else
        std::vector< molecule >::const_iterator itMol;  mol::molecule::const_cluster_iterator  itIso;
        std::tie( itIsot, itMol ) = a;
#endif
        auto idMol = std::distance( molecules.begin(), a.second );
        ms->setMass( idx, itIso->mass );
        if ( sp )
            ms->setTime( idx, sp->timeFromMass( itIso->mass ) );

        ms->setIntensity( idx, itIso->abundance * 100 );
        ms->setColor( idx, static_cast<unsigned>(idMol) % 17 );

        std::ostringstream o;
        if ( itMol->max_abundant_isotope() == itIso ) {
            o << ChemicalFormula::formatFormula( itMol->display_formula() )
              << " " << std::to_string( itIso->mass );
        } else {
            double delta = itIso->mass - itMol->max_abundant_isotope()->mass;
            int idelta = ( delta > 0 ) ? int( delta + 0.7 ) : int( delta - 0.7 );
            o << itMol->formula() << "(" << std::showpos << idelta << ")";
        }
        ms->get_annotations()
            << annotation( o.str(), itIso->mass, ms->intensity( idx ), int( idx ), 0, annotation::dataText );
        ++idx;
    }
    return ms;
}
