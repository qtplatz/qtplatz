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
#include "targeting/candidate.hpp"
#include "targetingmethod.hpp"
#include "chemicalformula.hpp"
#include "constants.hpp"
#include "massspectrum.hpp"
#include "molecule.hpp"
#include "msproperty.hpp"
#include "samplinginfo.hpp"
#include "segment_wrapper.hpp"
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

    class Targeting::impl {
    public:
        impl() {
        }
        impl( const impl& t ) : active_formula_( t.active_formula_ ) {
        }
        // typedef std::pair< double, std::string > adduct_type;
        typedef std::tuple< std::string    // active_formula (mol + adducts) --"M +[H]+"
                            , double       // exact mass for active_formula
                            , int          // charge
                            , std::string  // mol formula defined in moltable::data.formula (exclude adduct)
                            , std::string  // mol syonym
                            > active_formula_type;
        enum { active_formula_formula
            , active_formula_mass
            , active_formula_charge
            , active_formula_mol_formula
            , active_formula_synonym };
        std::vector< active_formula_type > active_formula_;
    };


    //////////////
    //////////////

    namespace targeting {

        typedef std::map< std::string, uint32_t > adduct_repeat_t; // formula, repeat

        std::string to_addlose_string( std::pair< mol::molecule, mol::molecule >&& addlose, int scharge ) {
            std::ostringstream o;
            if ( !std::get< 0 >( addlose ).elements().empty() ) { // has adduct
                const auto& add = std::get< 0 >( addlose );
                if ( std::get< 1 >( addlose ).elements().empty() ) // without loss
                    o << "+[" << add.formula( false ) << "]";
                else
                    o << "+" << add.formula( false );
            }
            if ( !std::get< 1 >( addlose ).elements().empty() ) { // has loss
                const auto& loss = std::get< 1 >( addlose );
                if ( (scharge < 0 ) && ( loss.elements().size() == 1 && loss.elements().at( 0 ).atomicNumber() == 1 ) ) { // H
                    // -H --> -[H]+ to handle polarity appropriately in ChemicalFormula class
                    scharge = (-scharge);
                }
                o << "-[" << loss.formula( false ) << "]";
            }
            if ( std::abs( scharge ) > 1 )
                o << std::abs( scharge );
            o << (scharge < 0 ? '-' : '+');
            return o.str();
        };

        std::pair< mol::molecule, mol::molecule > // add, sub, charge
        to_molecule( adduct_repeat_t&& alist ) {
            std::pair< adcontrols::mol::molecule, adcontrols::mol::molecule > addlose;
            for ( const auto& m: alist ) {
                auto v = adcontrols::ChemicalFormula::split( m.first ); // remove leading +|- (add/sub specifier)
                auto mol = adcontrols::ChemicalFormula::toMolecule( v.at(0).first ) * size_t( m.second );
                if ( v.at(0).second == '+' ) {
                    std::get<0>(addlose) += mol; // adduct
                } else {
                    std::get<1>(addlose) += mol; // lose
                }
            }
            return addlose;
        }

        std::vector< std::string >
        make_combination( uint32_t charge, const std::vector< std::string >& adducts, ion_polarity polarity )
        {
            std::vector< std::string > addloses;
            const size_t r = charge;
            const size_t n = adducts.size();
            const int scharge = polarity == polarity_positive ? charge : -int(charge);
            if ( n > 0 ) {
                std::vector< std::vector< std::string >::const_iterator > v_iter( r, adducts.begin() );
                do {
                    adduct_repeat_t an_adduct;
                    std::for_each( v_iter.begin(), v_iter.end(), [&]( const auto& it ){
                        an_adduct[ *it ]++;
                    });
                    addloses.emplace_back( to_addlose_string( to_molecule( std::move( an_adduct ) ), scharge ) );
                } while ( boost::next_mapping( v_iter.begin(), v_iter.end(), adducts.begin(), adducts.end() ) );
            }
            return addloses;
        }
    }
}

using namespace adcontrols;

Targeting::~Targeting()
{
}

Targeting::Targeting() : method_( std::make_shared< TargetingMethod >() )
                       , impl_( std::make_unique< impl >() )
{
}

Targeting::Targeting( const Targeting& t ) : method_( t.method_ )
                                           , candidates_( t.candidates_ )
                                           , impl_( std::make_unique< impl >( *t.impl_ ) )
{
}

Targeting::Targeting( const TargetingMethod& m ) : method_( std::make_shared<TargetingMethod>( m ) )
                                                 , impl_( std::make_unique< impl >() )
{
    setup( m );
}

bool
Targeting::find_candidate( const MassSpectrum& ms, int fcn, adcontrols::ion_polarity polarity )
{
    if ( ms.size() == 0 )
        return false;

    using adcontrols::ChemicalFormula;

    adcontrols::MSFinder finder( method_->tolerance( method_->toleranceMethod() ), method_->findAlgorithm(), method_->toleranceMethod() );

    for ( auto& aformula : impl_->active_formula_ ) {
        double exact_mass = std::get< impl::active_formula_mass >( aformula ); // search 'M'
        size_t pos = finder( ms, exact_mass );
        if ( pos != MassSpectrum::npos ) {
            double mass = ms.mass( pos );
            int scharge = std::get< impl::active_formula_charge >( aformula );
            auto synonym = std::get< impl::active_formula_synonym >( aformula );
            auto formula = std::get< impl::active_formula_formula >( aformula );

            std::string display_name;
            if ( !synonym.empty() && synonym.size() < 8 ) {
                display_name = synonym;
            } else {
                auto v = ChemicalFormula::split( formula );
                display_name = ( boost::format("%.1f")
                                 % adcontrols::ChemicalFormula().getMonoIsotopicMass( v.at(0).first, false ) ).str();
            }

            using namespace adcontrols::cf;
            display_name += " " + ChemicalFormula::formatFormulae( formula, {Charge{scharge}, RichText{true}, {"M"} } );

            //ADDEBUG() << "------------- Targeting::find_candidate: " << std::make_tuple( scharge, synonym, formula, display_name );
            candidates_.emplace_back( uint32_t( pos )
                                      , fcn
                                      , scharge
                                      , mass
                                      , exact_mass
                                      , std::get< impl::active_formula_formula >( aformula ) // full formula
                                      , synonym
                                      , display_name
                );
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
            tms.annotations().erase_if( [](const auto& a){ return a.flags() <= annotation::flag_targeting; } ); // clear existing annotations
        }

        adcontrols::MSFinder finder( method_->tolerance( method_->toleranceMethod() ), method_->findAlgorithm(), method_->toleranceMethod() );

        for ( auto& candidate : candidates_ ) {

            do {
                ////////////// isotope cluster match ///////////////
                // generate cluster pattern
                auto neutral = adcontrols::ChemicalFormula::neutralize( candidate.formula );

                // ADDEBUG() << "\t" << boost::json::value_from( candidate ) << ", neutral: " << neutral;

                auto v_peak =
                    isotopeCluster( 1.0e-6, 7000 )( ChemicalFormula::split( neutral.first ), neutral.second );
                auto bp =
                    std::max_element( v_peak.begin(), v_peak.end()
                                      , [](const auto& a, const auto& b){ return a.abundance < b.abundance; }); // base peak

                auto& tms = segment_wrapper<>( ms )[ candidate.fcn ];

                double error = tms.mass( candidate.idx ) - bp->mass;
                size_t nCarbons = ChemicalFormula::number_of_atoms( neutral.first, "C" );

                std::vector< targeting::isotope > isotopes( v_peak.size() );
                std::transform( v_peak.begin(), v_peak.end(), isotopes.begin()
                                , [](const auto& v){ return targeting::isotope(-1, 0, 0, 0, v.mass, v.abundance ); });

                // order of abundance desc
                std::sort( isotopes.begin(), isotopes.end()
                           , [](const auto& a, const auto& b){ return a.exact_abundance > b.exact_abundance; });

                // remove most abundant peak that is equivalent to candidate
                isotopes.erase( isotopes.begin() );

                for ( std::vector< targeting::isotope >::iterator it = isotopes.begin(); it < isotopes.end(); ++it ) {
                    auto& i = *it;
                    auto pos = finder( tms, it->exact_mass + error );

                    if ( pos != MassSpectrum::npos ) {
                        auto pIt = std::find_if( isotopes.begin(), it
                                                 , [pos]( const auto& a ){ return a.idx == pos; });
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
                std::sort( isotopes.begin(), isotopes.end()
                           , [](const auto& a, const auto& b){ return a.exact_mass < b.exact_mass; });
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

                tms.addAnnotation({
                        candidate.display_name // candidate.formula
                        , tms.mass( candidate.idx )
                        , tms.intensity( candidate.idx )
                        , int(candidate.idx)
                        , pri
                        , annotation::dataText // annotation::dataFormula
                        , annotation::flag_targeting } );

                auto list = adcontrols::ChemicalFormula::split( candidate.formula );
                double exactMass = adcontrols::ChemicalFormula().getMonoIsotopicMass( list ).first;
                std::ostringstream adducts;
                for ( size_t i = 1; i < list.size(); ++i )
                    adducts << list[i].second << list[i].first;

                adcontrols::annotation::reference_molecule ref( candidate.display_name
                                                                , std::get< 0 >( list[0] )
                                                                , adducts.str()
                                                                , exactMass
                                                                , tms.mass( candidate.idx )
                                                                , boost::json::value_from( candidate ));
                tms.addAnnotation( { boost::json::value_from( ref )
                        , tms.mass( candidate.idx ), tms.intensity( candidate.idx ), int( candidate.idx ), pri } );

                // annotate isotopes}
                std::string text = "*"; // ChemicalFormula::formatFormulae( candidate.formula, true ) + "*";
                for ( const auto& i: candidate.isotopes ) {
                    if ( std::abs( i.abundance_ratio_error ) < 0.5 ) { // anotate if abunance error < 50%
                        if ( i.idx >= 0 ) {
                            tms.setColor( i.idx, 16 ); // dark orange
                            int xpri = pri * i.exact_abundance;
                            tms.addAnnotation({
                                    text
                                    , tms.mass( i.idx )
                                    , tms.intensity( i.idx )
                                    , i.idx, xpri, annotation::dataText, annotation::flag_targeting } );
                        }
                    }
                }
            }
        }

        // erase lower score candidates
        candidates_.erase( std::remove_if( candidates_.begin(), candidates_.end()
                                           , [](const auto& c ){ return c.score < 0; }), candidates_.end() );

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
    static ChemicalFormula cf;

    impl_->active_formula_.clear();

    auto charge_range = m.chargeState();
    std::vector< std::string > addlose_global;

    const auto pol = m.molecules().polarity();

    // ADDEBUG() << "====== targeting setup ====== polarity ? "
    //           << (m.molecules().polarity() == polarity_positive ? "positive\t" : "negative\tcharge range: ")
    //           << charge_range;

    for ( auto& a: m.adducts( m.molecules().polarity() ) ) {
        if ( a.first ) {
            addlose_global.emplace_back( a.second );
        }
    }
    // <----

    for ( auto& x : m.molecules().data() ) {
        if ( x.enable() ) {
            // if moltable has a designated adduct
            if ( !std::string( x.adducts(pol) ).empty() ) {

                auto formula = x.formula() + x.adducts(pol);
                double mass; int scharge;
                std::tie( mass, scharge ) = cf.getMonoIsotopicMass( ChemicalFormula::split( x.formula() + x.adducts(pol) ), 0 );
                impl_->active_formula_.emplace_back( formula, mass, scharge, x.formula(), x.synonym() );

            } else if ( addlose_global.empty() ) { // historical infiTOF -- all global check is off
                auto formula = x.formula();
                for ( uint32_t charge = charge_range.first; charge <= charge_range.second; ++charge ) {
                    int scharge = m.molecules().polarity() == polarity_positive ? charge : -static_cast<int>(charge);
                    impl_->active_formula_.emplace_back(
                        formula
                        , cf.getMonoIsotopicMass( ChemicalFormula::split( formula ), scharge ).first
                        , scharge
                        , x.formula()
                        , x.synonym() );
                    ADDEBUG() << "###<" << charge << ">" << impl_->active_formula_.back();
                }
            } else {
                for ( uint32_t charge = charge_range.first; charge <= charge_range.second; ++charge ) {
                    int scharge = m.molecules().polarity() == polarity_positive ? charge : -static_cast<int>(charge);
                    for ( const auto& addlose: targeting::make_combination( charge, addlose_global, pol ) ) {
                        auto formula = x.formula() + addlose;
                        impl_->active_formula_.emplace_back(
                            formula
                            , cf.getMonoIsotopicMass( ChemicalFormula::split( formula ), scharge ).first
                            , scharge
                            , x.formula()
                            , x.synonym() );
                    }
                }
            }
        }
    }
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
