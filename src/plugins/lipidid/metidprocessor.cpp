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
#include "mol.hpp"
#include "simple_mass_spectrum.hpp"
#include <adwidgets/progressinterface.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/metidmethod.hpp>
#include <adfs/sqlite.hpp>

#include <adfs/get_column_values.hpp>
#include <adportable/debug.hpp>

using lipidid::MetIdProcessor;

namespace lipidid {
    class MetIdProcessor::impl {
    public:
        impl() {}
        impl( const adcontrols::MetIdMethod& m ) : method_( m ) {}

        adcontrols::MetIdMethod method_;
        std::map< std::string, std::vector< lipidid::mol > > mols_; // stdformula, vector< mol >
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
        sql.prepare( "SELECT id,formula,smiles,inchiKey FROM mols WHERE mass < 1200 ORDER BY mass" );
        while ( sql.step() == adfs::sqlite_row ) {
            (*progress)();
            ++counts;
            auto [ id, formula, smiles, inchikey ] = adfs::get_column_values< int64_t, std::string, std::string, std::string >( sql );
            impl_->mols_[ formula ].emplace_back( std::make_tuple( id, formula, smiles, inchikey ) );
        }
    }

    size_t total_size = impl_->mols_.size() * method.adducts().size();

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
    impl_->simple_mass_spectrum_ = std::move( tms );
    // ADDEBUG() << "\n" << boost::json::object{{ "simple_mass_spectrum", *impl_->simple_mass_spectrum_ }};

    if ( auto refMs = std::make_shared< adcontrols::MassSpectrum >() ) {
        std::vector< double > masses, intensities;
        refMs->clone( *ms, false );
        auto simple_ms( impl_->simple_mass_spectrum_ );
        for ( size_t idx = 0; idx < simple_ms->size(); ++idx ) {
            auto [ tof, mass, intensity, color ] = (*simple_ms)[ idx ];

            auto candidates = simple_ms->candidates( idx );
            if ( ! candidates.empty() ) {
                const auto& candidate = candidates.at( 0 );
                auto cluster = isoCluster::compute( candidate.formula, candidate.adduct );
                refMs->get_annotations()
                    << adcontrols::annotation( candidate.formula + " " + candidate.adduct
                                               , cluster.at(0).first // mass
                                               , cluster.at(0).second * intensity
                                               , masses.size() // index
                                               , cluster.at(0).second * intensity
                                               , adcontrols::annotation::dataFormula
                                               , adcontrols::annotation::flag_targeting );
                for ( const auto& ipk: cluster ) {
                    masses.emplace_back( ipk.first );
                    intensities.emplace_back( ipk.second * intensity );
                }
            }
        }
        refMs->setMassArray( std::move( masses ) );
        refMs->setIntensityArray( std::move( intensities ) );
        impl_->refms_ = std::move( refMs );
    }

    return tms;

    return {};
}
