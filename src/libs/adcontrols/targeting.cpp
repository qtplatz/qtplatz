/**************************************************************************
** Copyright (C) 2010-2019 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2019 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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
#include "isotopecluster.hpp"
#include "targeting.hpp"
#include "targetingmethod.hpp"
#include "chemicalformula.hpp"
#include "constants.hpp"
#include "massspectrum.hpp"
#include "molecule.hpp"
#include "msproperty.hpp"
#include "samplinginfo.hpp"
#include <adcontrols/moltable.hpp>
#include <adportable/debug.hpp>
#include <adportable/combination.hpp>
#include <adportable/for_each_combination.hpp>
#include <adportable_serializer/portable_binary_oarchive.hpp>
#include <adportable_serializer/portable_binary_iarchive.hpp>
#include <boost/format.hpp>
#include <algorithm>
#include <iterator>
#include <numeric>
#include <sstream>
#include <set>
#include <map>

namespace adcontrols {

    typedef std::map< std::string, uint32_t > adduct_complex_t; // adduct[replicates] such as (CH3CN)2, ...

    struct make_formula {
        template< char sign >
        bool to_formula( int charge
                         , const adduct_complex_t& complex
                         , const std::map< std::string, adcontrols::ChemicalFormula::formula_adduct_t >& adducts
                         , std::ostream& o ) {

            bool once_flag(true);

            for ( const auto& a: complex ) {
                assert( a.second );
                auto it = adducts.find( a.first );
                if ( it != adducts.end() ) {
                    if ( it->second.second == sign ) {
                        if ( once_flag ) {
                            o << sign;
                            if ( charge )
                                o << "[";
                            once_flag = false;
                        }
                        size_t natoms = adcontrols::ChemicalFormula::number_of_atoms( it->second.first );
                        if ( a.second > 1 && natoms > 1 )
                            o << "(";
                        o << it->second.first;
                        if ( a.second > 1 && natoms > 1 )
                            o << ")";
                        if ( a.second > 1 )
                            o << a.second;
                    }
                }
            }

            if ( charge && !once_flag ) {
                o << "]";
                if ( std::abs( charge ) > 1 )
                    o << std::abs( charge );
                o << (charge > 0 ? '+' : '-');
            }
            return !once_flag;
        }

        std::string
        operator()( int charge
                    , const adduct_complex_t& complex
                    , const std::map< std::string, adcontrols::ChemicalFormula::formula_adduct_t >& adducts ) {
            std::ostringstream o;
            to_formula<'-'>( 0, complex, adducts, o );
            if ( ! to_formula<'+'>( charge, complex, adducts, o ) ) {
                std::ostringstream n;
                to_formula<'-'>( charge, complex, adducts, n );
                return n.str();
            }
            return o.str();
        }

    };

    struct make_combination {

        typedef std::map< std::string, uint32_t > adduct_complex_t;
        std::vector< adduct_complex_t > complex_;
#if 0
        static void print( int charge
                           , const adduct_complex_t& complex
                           , const std::map< std::string, adcontrols::ChemicalFormula::formula_adduct_t >& adducts
                           , std::ostream& o ) {
            o << make_formula()( charge, complex, adducts );
        }
#endif

        std::vector< std::string >
        operator()( uint32_t charge
                    , const std::map< std::string /*key*/, adcontrols::ChemicalFormula::formula_adduct_t >& adducts /*formula,[+|-]*/) {

            const size_t r = charge;
            const size_t n = adducts.size();

            if ( n > 0 ) {
                std::vector< std::string > v_keys( n );
                std::transform( adducts.begin(), adducts.end(), v_keys.begin(), [](const auto& a){ return a.first; });
                std::vector< std::vector< std::string >::const_iterator > v_iter( r, v_keys.begin() );
                do {
                    std::ostringstream o;

                    adduct_complex_t an_adduct;
                    std::for_each( v_iter.begin(), v_iter.end(), [&]( const auto& it ){ an_adduct[ *it ]++; });
                    complex_.emplace_back( std::move( an_adduct ) );

                } while ( boost::next_mapping( v_iter.begin(), v_iter.end(), v_keys.begin(), v_keys.end() ) );

                std::vector< std::string > results;
                for ( const auto& complex: complex_ )
                    results.emplace_back( make_formula()( charge, complex, adducts ) );

                return results;
            }
            return std::vector< std::string >();
        }

    };
}

using namespace adcontrols;

Targeting::Targeting() : method_( std::make_shared< TargetingMethod >() )
{
}

Targeting::Targeting( const Targeting& t ) : method_( t.method_ )
                                           , candidates_( t.candidates_ )
                                           , active_formula_( t.active_formula_ )
{
}

Targeting::Targeting( const TargetingMethod& m ) : method_( std::make_shared<TargetingMethod>( m ) )
{
    setup( m );
}

Targeting::Candidate::Candidate() : idx(0)
                                  , fcn(0)
                                  , charge(1)
                                  , mass(0)
                                  , exact_mass(0)
                                  , score(0)
{
}

Targeting::Candidate::Candidate( const Candidate& t ) : idx( t.idx )
                                                      , fcn( t.fcn )
                                                      , charge( t.charge )
                                                      , mass( t.mass )
                                                      , formula( t.formula )
                                                      , exact_mass( t.exact_mass )
                                                      , score( t.score )
                                                      , isotopes( t.isotopes )
{
}


Targeting::Candidate::Candidate( uint32_t _idx, uint32_t _fcn, int32_t _charge, double _mass, double _exact_mass, const std::string& _formula )
    : idx( _idx )
    , fcn( _fcn )
    , charge( _charge )
    , mass( _mass )
    , formula( _formula )
    , exact_mass( _exact_mass )
    , score( 0 )
{
}

bool
Targeting::find_candidate( const MassSpectrum& ms, int fcn, adcontrols::ion_polarity polarity )
{
    if ( ms.size() == 0 )
        return false;

    adcontrols::MSFinder finder( method_->tolerance( method_->toleranceMethod() ), method_->findAlgorithm(), method_->toleranceMethod() );

    for ( auto& formula : active_formula_ ) {
        double exact_mass = formula.second; // search 'M'
        size_t pos = finder( ms, exact_mass );
        if ( pos != MassSpectrum::npos ) {
            double mass = ms.mass( pos );
            auto neutral = adcontrols::ChemicalFormula::neutralize( formula.first );
            candidates_.emplace_back( uint32_t( pos /*idx*/ ), fcn, neutral.second/*charge*/, mass, exact_mass, formula.first /*formula*/ );
        }
    }
    return true;
}

bool
Targeting::operator()( const MassSpectrum& ms )
{
    candidates_.clear();

    if ( !ms.isCentroid() )
        return false;

    // bool polarity_positive = (ms.polarity() == PolarityPositive) || (ms.polarity() == PolarityIndeterminate);
    auto polarity = method_->molecules().polarity();

    segment_wrapper< const adcontrols::MassSpectrum > segs( ms );
    int fcn = 0;
    for ( auto& fms : segs ) {
        find_candidate( fms, fcn++, polarity );
    }
    return true;
}

bool
Targeting::operator()( MassSpectrum& ms )
{
    if ( (*this)( static_cast< const MassSpectrum& >( ms ) ) ) {

        // erase existing annotations & colors
        for ( auto& tms: segment_wrapper<>( ms ) ) {
            tms.setColorArray( std::vector< uint8_t >() ); // clear color array
            tms.get_annotations().erase_if( [](const auto& a){ return a.flags() <= annotation::flag_targeting; } ); // clear existing annotations
        }

        adcontrols::MSFinder finder( method_->tolerance( method_->toleranceMethod() ), method_->findAlgorithm(), method_->toleranceMethod() );

        for ( auto& candidate : candidates_ ) {
            do {
                ////////////// isotope cluster match ///////////////
                // generate cluster pattern
                auto neutral = adcontrols::ChemicalFormula::neutralize( candidate.formula );
#if !defined NDEBUG && 0
                ADDEBUG() << "candidate.formula: " << candidate.formula << ", neutral: " << neutral;
#endif
                auto v_peak = isotopeCluster( 1.0e-6, 7000 )( ChemicalFormula::split( neutral.first ), neutral.second );
                auto bp = std::max_element( v_peak.begin(), v_peak.end(), [](const auto& a, const auto& b){ return a.abundance < b.abundance; }); // base peak

                auto& tms = segment_wrapper<>( ms )[ candidate.fcn ];

                double error = tms.mass( candidate.idx ) - bp->mass;
                size_t nCarbons = ChemicalFormula::number_of_atoms( neutral.first, "C" );

                std::vector< isotope > isotopes( v_peak.size() );
                std::transform( v_peak.begin(), v_peak.end(), isotopes.begin(), [](const auto& v){ return isotope(-1, 0, 0, 0, v.mass, v.abundance ); });

                // order of abundance desc
                std::sort( isotopes.begin(), isotopes.end(), [](const auto& a, const auto& b){ return a.exact_abundance > b.exact_abundance; });

                // remove most abundant peak that is equivalent to candidate
                isotopes.erase( isotopes.begin() );

                for ( std::vector< isotope >::iterator it = isotopes.begin(); it < isotopes.end(); ++it ) {
                    auto& i = *it;
                    auto pos = finder( tms, it->exact_mass + error );

                    if ( pos != MassSpectrum::npos ) {
                        auto pIt = std::find_if( isotopes.begin(), it, [pos]( const auto& a ){ return a.idx == pos; });
                        if ( pIt == it ) { // if not already exists
                            i.idx = pos;
                            i.mass = tms.mass( pos ) - error; // base peak locked
                            i.abundance_ratio = tms.intensity( pos ) / tms.intensity( candidate.idx );
                            i.abundance_ratio_error = (i.abundance_ratio - i.exact_abundance)/i.exact_abundance;
#if !defined NDEBUG
                            ADDEBUG() << pos << ":\t" << candidate.formula
                                      << boost::format("\texact_mass: %.4lf (%.1f)" ) % i.exact_mass % (error*100)
                                      << boost::format(",\tmass error(mDa): %.5lf" ) % ((tms.mass( pos ) - (i.mass + error)) * 1000)
                                      << boost::format(",\tabundance: %.5g,\t%.5g") % i.exact_abundance % i.abundance_ratio
                                      << boost::format(",\terror: %g") % ((100*(i.abundance_ratio - i.exact_abundance))/i.exact_abundance);
#endif
                        }
                    }
                }

                (void)nCarbons; // ignore isotope detection check
                std::sort( isotopes.begin(), isotopes.end(), [](const auto& a, const auto& b){ return a.exact_mass < b.exact_mass; });
                auto tail = std::find_if( isotopes.rbegin(), isotopes.rend(), [](const auto& a){ return a.idx >= 0; } );
                isotopes.erase( tail.base(), isotopes.end() );

                candidate.isotopes = std::move( isotopes );

            } while(0);

            ///////////////////
            // following annotation may be overwrote from adwidgets::mspeaktable through annotation_updatore via setPeakInfo
            if ( candidate.score >= 0 ) {
                auto& tms = segment_wrapper<>( ms )[ candidate.fcn ];
                tms.setColor( candidate.idx, 16 ); // dark orange
                int pri = 1000 * (std::log10( tms.intensity( candidate.idx ) / tms.maxIntensity() ) + 15); // 0..15000
                tms.get_annotations()
                    << annotation( candidate.formula, tms.mass( candidate.idx )
                                   , tms.intensity( candidate.idx ), candidate.idx, pri, annotation::dataFormula, annotation::flag_targeting );
                // annotate isotopes
                std::string text = "*"; // ChemicalFormula::formatFormulae( candidate.formula, true ) + "*";
                for ( const auto& i: candidate.isotopes ) {
                    if ( std::abs( i.abundance_ratio_error ) < 0.5 ) { // anotate if abunance error < 50%
                        if ( i.idx >= 0 ) {
                            tms.setColor( i.idx, 16 ); // dark orange
                            int xpri = pri * i.exact_abundance;
                            tms.get_annotations()
                                << annotation( text, tms.mass( i.idx ), tms.intensity( i.idx ), i.idx, xpri, annotation::dataText, annotation::flag_targeting );
                        }
                    }
                }
            }
        }

        // erase lower score candidates
        candidates_.erase( std::remove_if( candidates_.begin(), candidates_.end(), [](const auto& c ){ return c.score < 0; }), candidates_.end() );

        return true;
    }
    return false;
}

// Call from quanchromatogramprocessor
bool
Targeting::force_find( const MassSpectrum& ms, const std::string& formula, int32_t fcn )
{
    candidates_.clear();

    if ( !ms.isCentroid() )
        return false;
    if ( ms.size() == 0 )
        return false;

    auto neutral = ChemicalFormula::neutralize( formula );
    int charge = neutral.second ? neutral.second : 1;
    double exact_mass = ChemicalFormula().getMonoIsotopicMass( ChemicalFormula::split( formula ) ).first;

    if ( ms.nProtocols() <= fcn ) {
        ADDEBUG() << "Error: force_find: -- specified protocol " << fcn << " does not exist.";
        return false;
    }
    auto& fms = segment_wrapper< const adcontrols::MassSpectrum >( ms )[ fcn ];
    double width = method_->tolerance( idToleranceDaltons );
    do {
        adcontrols::MSFinder finder( width, idFindClosest );
        size_t pos = finder( fms, exact_mass );
        if ( pos != MassSpectrum::npos ) {
            candidates_.emplace_back( pos, fcn, charge, fms.mass( pos ), exact_mass, formula );
            return true;
        }
        width += width;
    } while ( width < 1.0 );

    return false;
}


void
Targeting::setup( const TargetingMethod& m )
{
    ChemicalFormula formula_parser;

    ADDEBUG() << "=================== targeting setup ======================";
    active_formula_.clear();

    std::map< std::string, adcontrols::ChemicalFormula::formula_adduct_t > adducts_global;

    bool positive = m.molecules().polarity() == adcontrols::polarity_positive;

    auto charge_range = m.chargeState();

    for ( auto& a: m.adducts( positive ) ) {
        if ( a.first ) { // enable
            for ( const auto& adduct: ChemicalFormula::split( a.second ) ) {
                auto sign = (adduct.second == '-' ? "-" : "+");
                auto pair = ChemicalFormula::neutralize( adduct.first ); // neutral formula, charge
                adducts_global[ sign + ChemicalFormula::standardFormula( pair.first, true ) ] = { pair.first, sign[0] };
            }
        }
    }

    for ( auto& x : m.molecules().data() ) {
        if ( x.enable() ) {

            std::map< std::string, adcontrols::ChemicalFormula::formula_adduct_t > adducts_local( adducts_global );

            if ( !std::string( x.adducts() ).empty() ) {
                auto formula = x.formula() + x.adducts();
                double mass; int charge;
                std::tie( mass, charge ) = formula_parser.getMonoIsotopicMass( ChemicalFormula::split( x.formula() + x.adducts() ), 0 );
                active_formula_.emplace_back( formula, mass );
                // todo
                // if adduct has ',' or ';' then multiple adducts combination should be added separately
                // for ( const auto& a: ChemicalFormula::split( x.adducts() ) ) {
                //     auto formula = std::string( x.formula() ) + (a.second == '-' ? "-" : "+") + a.first;
                //     double mass;
                //     std::tie( mass, std::ignore ) = formula_parser.getMonoIsotopicMass( ChemicalFormula::split( formula ), 0 );
                //     active_formula_.emplace_back( formula, mass );
                // }
            } else {
                for ( uint32_t charge = charge_range.first; charge <= charge_range.second; ++charge ) {
                    int icharge = positive ? charge : -static_cast<int>(charge);
                    if ( charge == 1 ) { // no charge 0 supported
                        if ( adducts_local.empty() ) {
                            std::ostringstream t;
                            t << "[" << x.formula() << "]" << ( positive ? '+' : '-' );
                            active_formula_.emplace_back( t.str(), formula_parser.getMonoIsotopicMass( ChemicalFormula::split( x.formula() ), icharge ).first );
                        } else {
                            for ( const auto& a: adducts_local ) {
                                std::ostringstream t;
                                t << x.formula() << a.second.second << "[" << a.second.first << "]+";  // "+|-" + ['adduct']+
                                active_formula_.emplace_back( t.str(), formula_parser.getMonoIsotopicMass( ChemicalFormula::split( t.str() ), icharge ).first );
                            }
                        }
                    } else if ( charge >= 2 ) {
                        if ( adducts_local.empty() ) {
                            std::ostringstream t;
                            t << "[" << x.formula() << "]" << charge << ( positive ? '+' : '-' );
                            active_formula_.emplace_back( t.str(), formula_parser.getMonoIsotopicMass( ChemicalFormula::split( x.formula() ), icharge ).first );
                        } else {
                            for ( const auto& a: make_combination()( charge, adducts_local ) ) {
                                std::ostringstream t;
                                t << x.formula() << a;
                                active_formula_.emplace_back( t.str(), formula_parser.getMonoIsotopicMass( ChemicalFormula::split( t.str() ), icharge ).first );
                            }
                        }
                    }
                }
            }
        }
    }
}

//static
std::vector< std::tuple<std::string, double, int> >
Targeting::make_mapping( const std::pair<uint32_t, uint32_t>& charge_range
                         , const std::string& formula
                         , const std::string& adducts
                         , adcontrols::ion_polarity polarity )
{
    std::vector< std::tuple<std::string, double, int> > res;

    std::map< std::string, adcontrols::ChemicalFormula::formula_adduct_t > adducts_local;

    for ( const auto& adduct: ChemicalFormula::split( adducts ) ) {
        auto sign = (adduct.second == '-' ? "-" : "+");
        adducts_local[ sign + ChemicalFormula::standardFormula( adduct.first, true ) ] = { ChemicalFormula::neutralize(adduct.first).first, sign[0] };
    }

    for ( uint32_t charge = charge_range.first; charge <= charge_range.second; ++charge ) {
        int icharge = polarity == adcontrols::polarity_positive ? charge : -static_cast<int>(charge);
        if ( charge == 0 || charge == 1 ) {
            if ( adducts_local.empty() ) {
                if ( charge == 0 ) {
                    double mass; int fcharge;
                    std::tie( mass, fcharge) = ChemicalFormula().getMonoIsotopicMass( ChemicalFormula::split( formula ) );
                    res.emplace_back( formula, mass, fcharge ); // ChemicalFormula().getMonoIsotopicMass( ChemicalFormula::split( formula ), icharge ), icharge );
                } else {
                    std::ostringstream t;
                    t << "[" << formula << "]" << (icharge > 0 ? '+' : '-');
                    res.emplace_back( t.str(), ChemicalFormula().getMonoIsotopicMass( ChemicalFormula::split( t.str() ), icharge ).first, icharge );
                }
            } else {
                for ( const auto& a: adducts_local ) {
                    std::ostringstream t;
                    t << formula << a.second.second << "[" << a.second.first << "]+";  // "+|-" + ['adduct']+
                    double mass = ChemicalFormula().getMonoIsotopicMass( ChemicalFormula::split( t.str() ), icharge ).first;
                    res.emplace_back( t.str(), mass, icharge );
                }
            }
        } else if ( charge >= 2 ) {
            if ( adducts_local.empty() ) {
                std::ostringstream t;
                t << "[" << formula << "]" << charge << (icharge > 0 ? '+' : '-');
                res.emplace_back( t.str(), ChemicalFormula().getMonoIsotopicMass( ChemicalFormula::split( t.str() ), icharge ).first, icharge );
            } else {
                for ( const auto& a: make_combination()( charge, adducts_local ) ) {
                    std::ostringstream t;
                    t << formula << a;
                    double mass = ChemicalFormula().getMonoIsotopicMass( ChemicalFormula::split( t.str() ), icharge ).first;
                    res.emplace_back( t.str(), mass, icharge );
                }
            }
        }
    }

    return res;
}

bool
Targeting::archive( std::ostream& os, const Targeting& v )
{
    portable_binary_oarchive ar( os );
    ar << v;
    return true;
}

bool
Targeting::restore( std::istream& is, Targeting& v )
{
    portable_binary_iarchive ar( is );
    ar >> v;
    return true;
}
