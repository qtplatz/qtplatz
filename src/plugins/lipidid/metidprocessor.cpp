/**************************************************************************
** Copyright (C) 2022-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2022-2022 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#include "metidprocessor.hpp"
#include "candidate.hpp"
#include "document.hpp"
#include "mol.hpp"
#include "simple_mass_spectrum.hpp"
#include "isocluster.hpp"
#include "isopeak.hpp"
#include <adcontrols/annotation.hpp>
#include <adcontrols/annotations.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/metidmethod.hpp>
#include <adfs/get_column_values.hpp>
#include <adportable/debug.hpp>
#include <adwidgets/progressinterface.hpp>

using lipidid::MetIdProcessor;

namespace lipidid {

    struct reference_mass {
        typedef std::tuple< double, int, std::string, std::string > value_type;
        value_type t_;
        reference_mass( value_type&& t ) : t_( std::move( t ) ) {}
        reference_mass( const reference_mass& t ) : t_( t.t_ ) {}
        double mass() const { return std::get< 0 >( t_ ); }
        int charge() const { return std::get< 1 >( t_ ); }
        std::string formula() const { return std::get< 2 >( t_ ); }
        std::string adducts() const { return std::get< 3 >( t_ ); }
    };

    struct make_reference_spectrum {
        const std::map< std::string, std::vector< std::shared_ptr< lipidid::mol > > >& mols_;
        make_reference_spectrum( std::map< std::string, std::vector< std::shared_ptr< lipidid::mol > > >& mols ) : mols_( mols ) {}
        std::shared_ptr< adcontrols::MassSpectrum > operator()(const adcontrols::MassSpectrum&
                                                               , const simple_mass_spectrum& );
    };

    class MetIdProcessor::impl {
    public:
        impl() {}
        impl( const adcontrols::MetIdMethod& m ) : method_( m ) {}

        std::vector< std::string >
        getInChIKeys( const std::string& formula ) const {
            std::vector< std::string > t;
            auto it = mols_.find( formula );
            if ( it != mols_.end() ) {
                for ( const auto& mol: it->second )
                    t.emplace_back( mol->inchikey() );
            }
            return t;
        }

        void make_reference_spectrum();

        adcontrols::MetIdMethod method_;
        std::map< std::string, std::vector< std::shared_ptr< lipidid::mol > > > mols_; // stdformula, vector< mol >
        std::shared_ptr< const adcontrols::MassSpectrum > ms_;
        std::vector< reference_mass > reference_list_;
    };
}


MetIdProcessor::~MetIdProcessor()
{
}

MetIdProcessor::MetIdProcessor() : impl_( std::make_unique< impl >() )
{
}

MetIdProcessor::MetIdProcessor( const adcontrols::MetIdMethod& m ) : impl_( std::make_unique< impl >( m ) )
{
}

std::tuple< std::shared_ptr< const adcontrols::MassSpectrum > // acquired spectrum
            , std::shared_ptr< adcontrols::MassSpectrum > // reference (calculated) spectrum
            , std::shared_ptr< lipidid::simple_mass_spectrum > // reference (calculated) spectrum
            >
MetIdProcessor::find_all( adfs::sqlite& db
                          , std::shared_ptr< const adcontrols::MassSpectrum > ms
                          , std::shared_ptr< adwidgets::ProgressInterface > progress )
{
    auto self( shared_from_this() ); // protecting
    const auto& method = impl_->method_;
    double mass_tolerance = method.tolerance( method.toleranceMethod() );
    ADDEBUG() << "mass tolerance: " << mass_tolerance * 1000 << "mDa";

    using lipidid::simple_mass_spectrum;
    using lipidid::mass_value_t;
    auto tms = std::make_shared< lipidid::simple_mass_spectrum >();
    tms->populate( *ms, [](auto value){ return mass_value_t::color( value ) == 15; });
    if ( tms->size() == 0 ) {
        ADDEBUG() << "no colored peak.";
        return {};
    }

    size_t counts(0);
    if ( impl_->mols_.empty() ) {
        adfs::stmt sql( db );
        sql.prepare( "SELECT COUNT(*) form mols WHERE mass < 1200" );
        if ( sql.step() == adfs::sqlite_row ) {
            auto nmols = sql.get_column_value< int64_t >( 0 );
            (*progress)( 0, nmols + tms->size() );
        }

        ADDEBUG() << "populating mols from database...";
        sql.prepare( "SELECT id,formula,smiles,inchiKey,SlogP FROM mols WHERE mass < 1200 ORDER BY mass" );
        while ( sql.step() == adfs::sqlite_row ) {
            (*progress)();
            ++counts;
            auto [ id, formula, smiles, inchikey, SlogP ] = adfs::get_column_values< int64_t, std::string, std::string, std::string, double >( sql );
            auto mol = std::make_shared< lipidid::mol >( std::make_tuple( id, formula, smiles, inchikey, SlogP ) );
            moldb::instance() << mol;
            impl_->mols_[ formula ].emplace_back( mol );
        }
    }

    ADDEBUG() << impl_->mols_.size() << " formulae loaded from " << counts << " total molecules";
    ADDEBUG() << "generating reference mass list...";

    for ( auto it = impl_->mols_.begin(); it != impl_->mols_.end(); ++it ) {
        const auto& stdformula = it->first;
        for ( auto adduct: method.adducts() ) {
            if ( adduct.first ) { // enable
                auto formulae = adcontrols::ChemicalFormula::split( stdformula + " " + adduct.second );
                auto [mass, charge] = adcontrols::ChemicalFormula().getMonoIsotopicMass( formulae );
                impl_->reference_list_.emplace_back( std::make_tuple( mass, charge, stdformula, adduct.second ) );
            }
        }
    }

    ADDEBUG() << impl_->reference_list_.size() << " total number of masses.";

    // make it ascending order
    std::sort( impl_->reference_list_.begin(), impl_->reference_list_.end()
               , [](const auto& a, const auto& b){ return a.mass() < b.mass(); } );

    // iterate observed peaks in the simple_mass_spectrum
    for ( auto mIt = tms->begin(); mIt != tms->end(); ++mIt ) {
        (*progress)();
        auto observed_mass = mass_value_t::mass( *mIt );
        auto lit = std::lower_bound( impl_->reference_list_.begin()
                                     , impl_->reference_list_.end()
                                     , observed_mass - mass_tolerance / 2.0
                                     , []( const auto& a, double b ){ return a.mass() < b; });

        std::vector< std::vector< reference_mass >::const_iterator > tmp;
        if ( lit != impl_->reference_list_.end() ) {
            while ( lit->mass() < (observed_mass + mass_tolerance / 2.0) )
                tmp.emplace_back( lit++ );
        }
        // sort gathered references in abs(mass error) ascending order
        std::sort( tmp.begin(), tmp.end()
                   , [&](const auto& a, const auto& b){
                       return std::abs( observed_mass - a->mass() ) < std::abs( observed_mass - b->mass() );
                   });
        const size_t index = std::distance( tms->begin(), mIt ); // peak index

        for ( const auto& t: tmp ) {
            // isotope cluster
            double mass         = t->mass();
            std::string formula = t->formula();
            std::string adducts = t->adducts();

            auto cluster = isoCluster::compute( formula // ion_mass( *tIstd::get< 1 >( *tIt )->first // formula
                                                , adducts     // *std::get< 2 >( *cIt )     // adduct
                                                , 1.0e-4      // abundance low limit
                                                , 4000 );     // R.P.

            candidate x( mass               // exact mass
                         , formula          // formula := map[string, mol]
                         , adducts          // adduct
                         , mass - mass_value_t::mass( (*tms)[ index ] ) // mass error
                         , tms->find_cluster( index, cluster )
                         , impl_->getInChIKeys( formula )
                );
            tms->add_a_candidate( index, std::move( x ) );
        }
    }
    // impl_->simple_mass_spectrum_ = std::move( tms );
    auto refms = make_reference_spectrum( impl_->mols_ )( *ms, *tms );
    return { ms, refms, tms };
    // ADDEBUG() << "\n" << boost::json::object{{ "simple_mass_spectrum", *impl_->simple_mass_spectrum_ }};
}

std::shared_ptr< adcontrols::MassSpectrum > // reference (calculated) spectrum
MetIdProcessor::compute_reference_spectrum( const std::string& formula // contains adducts
                                            , double abundance
                                            , std::shared_ptr< const adcontrols::MassSpectrum > ms )
{
    auto refMs = std::make_shared< adcontrols::MassSpectrum >();
    refMs->clone( *ms, false );
    auto cluster = isoCluster::compute( formula );
    std::vector< double > masses, intensities;
    std::vector< uint8_t > colors;
    refMs->get_annotations()
        << adcontrols::annotation( formula
                                   , cluster.at(0).first // mass
                                   , cluster.at(0).second * abundance
                                   , masses.size() // index
                                   , cluster.at(0).second * abundance
                                   , adcontrols::annotation::dataFormula
                                   , adcontrols::annotation::flag_targeting );
    for ( const auto& ipk: cluster ) {
        masses.emplace_back( ipk.first );
        intensities.emplace_back( ipk.second * abundance );
        colors.emplace_back( 4 ); // deep pink
    }
    refMs->setMassArray( std::move( masses ) );
    refMs->setIntensityArray( std::move( intensities ) );
    refMs->setColorArray( std::move( colors ) );
    return refMs;
}


namespace lipidid {

    std::shared_ptr< adcontrols::MassSpectrum >
    make_reference_spectrum::operator()(const adcontrols::MassSpectrum& ms
                                        , const simple_mass_spectrum& simple_ms )
    {
        auto refMs = std::make_shared< adcontrols::MassSpectrum >();
        std::vector< double > masses, intensities;
        std::vector< uint8_t > colors;
        refMs->clone( ms, false );
        int cid(0);

        for ( size_t idx = 0; idx < simple_ms.size(); ++idx ) {
            auto [ tof, mass, intensity, color ] = simple_ms[ idx ];

            auto candidates = simple_ms.candidates( idx );

            if ( ! candidates.empty() ) {
                const auto& candidate = candidates.at( 0 );
                auto cluster = isoCluster::compute( candidate.formula(), candidate.adduct() );
                refMs->get_annotations()
                    << adcontrols::annotation( candidate.formula() + " " + candidate.adduct()
                                               , cluster.at(0).first // mass
                                               , cluster.at(0).second * intensity
                                               , masses.size() // index
                                               , cluster.at(0).second * intensity
                                               , adcontrols::annotation::dataFormula
                                               , adcontrols::annotation::flag_targeting );
                for ( const auto& ipk: cluster ) {
                    masses.emplace_back( ipk.first );
                    intensities.emplace_back( ipk.second * intensity );
                    colors.emplace_back( cid & 1 ? 0 : 6 ); // blue, indigo
                }
                ++cid;
            }
        }
        refMs->setMassArray( std::move( masses ) );
        refMs->setIntensityArray( std::move( intensities ) );
        refMs->setColorArray( std::move( colors ) );
        return refMs;
    }

}
