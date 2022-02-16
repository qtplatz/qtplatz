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

#include "document.hpp"
#include <adcontrols/massspectrum.hpp>
#include <adportable/debug.hpp>
#include <adportfolio/folium.hpp>
#include <adportfolio/folder.hpp>
#include <adfs/sqlite.hpp>
#include <qtwrapper/settings.hpp>
#include <app/app_version.h> // <-- for Core::Constants::IDE_SETTINGSVARIANT_STR
#include <QMessageBox>
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <boost/filesystem.hpp>
#include <tuple>

Q_DECLARE_METATYPE( portfolio::Folium )

namespace lipidid {

    struct user_preference {
        static boost::filesystem::path path( QSettings * settings ) {
            boost::filesystem::path dir( settings->fileName().toStdWString() );
            return dir.remove_filename() / "lipidid";
        }
    };

    class document::impl {
    public:
        ~impl() {
            ADDEBUG() << "## lipidid::document::impl DTOR ##";
        }
        impl() : settings_( std::make_unique< QSettings >( QSettings::IniFormat
                                                           , QSettings::UserScope
                                                           , QLatin1String( Core::Constants::IDE_SETTINGSVARIANT_STR )
                                                           , QLatin1String( "lipidid" ) ) )   {
        }
        std::unique_ptr< QSettings > settings_;
        QSqlDatabase db_;
        std::shared_ptr< const adcontrols::MassSpectrum > ms_;
    };


}

using lipidid::document;

document::~document()
{
    ADDEBUG() << "## lipidid::document DTOR ##";
}

document::document(QObject *parent) : QObject(parent)
                                    , impl_( std::make_unique< impl >() )
{
}

document *
document::instance()
{
    static document __instance(0);
    return &__instance;
}

QSettings *
document::settings()
{
    return instance()->impl_->settings_.get();
}

void
document::initialSetup()
{
    static std::atomic_int counts = 0;
    if ( counts++ ) {
        ADDEBUG() << "## " << __FUNCTION__ << " ## " << counts;
        return;
    }
    do {
        boost::filesystem::path dir = user_preference::path( impl_->settings_.get() );
        if ( !boost::filesystem::exists( dir ) ) {
            if ( !boost::filesystem::create_directories( dir ) ) {
                QMessageBox::information( 0, "lipidid::document"
                                          , QString( "Work directory '%1' can not be created" ).arg( dir.string().c_str() ) );
            }
        }
    } while ( 0 );

    do {
        auto path = boost::filesystem::path( impl_->settings_->fileName().toStdString() );
        auto dir = path.remove_filename() / "lipidid";
        boost::filesystem::path fpath = qtwrapper::settings( *impl_->settings_ ).recentFile( "LIPID_MAPS", "Files" ).toStdWString();
        if ( fpath.empty() ) {
            fpath = dir / "lipid_maps.db";
        }
        if ( boost::filesystem::exists( fpath ) ) {
            if ( fpath.extension() == "adfs" || fpath.extension() == "db" ) {
                auto db = std::make_shared< adfs::sqlite >();
                db->open( fpath.string().c_str(), adfs::readonly );
                adfs::stmt sql(*db);
                if ( sql.prepare( "SELECT name FROM sqlite_master WHERE type='table' AND name='mols'" ) ) {
                    if ( sql.step() != adfs::sqlite_row ) {
                        ADDEBUG() << "empty database";
                    }
                }
            }
            QSqlDatabase db = QSqlDatabase::addDatabase( "QSQLITE", "LIPID_MAPS" );
            db.setDatabaseName( QString::fromStdString( fpath.string() ) );
            if ( db.open() ) {
                impl_->db_ = std::move( db );
                emit onConnectionChanged();
            } else {
                QMessageBox::critical(0
                                      , QObject::tr("Cannot open database")
                                      , QObject::tr("Unable to establish a database connection.\nClick Cancel to exit.")
                                      , QMessageBox::Cancel );
            }
        } else {
            ADDEBUG() << fpath << " not exists";
            return;
        }
    } while ( 0 );

}

void
document::finalClose()
{
}

QSqlDatabase
document::sqlDatabase()
{
    return impl_->db_;
}

void
document::handleAddProcessor( adextension::iSessionManager *, const QString& file )
{
    ADDEBUG() << "## " << __FUNCTION__ << "\t" << file.toStdString();
}

// change node (folium) selection
void
document::handleSelectionChanged( adextension::iSessionManager *
                                  , const QString& file, const portfolio::Folium& folium )
{
    using portfolio::is_any_shared_of;
    if ( is_any_shared_of< adcontrols::MassSpectrum, const adcontrols::MassSpectrum >( folium ) ) {
        using portfolio::get_shared_of;
        if ( auto ptr = get_shared_of< const adcontrols::MassSpectrum, adcontrols::MassSpectrum >()( folium.data() ) ) {
            if ( ptr->isCentroid() ) {
                impl_->ms_ = ptr;
                emit dataChanged( folium );
            }
        }
    }
}

// data contents changed
void
document::handleProcessed( adextension::iSessionManager *
                           , const QString& file, const portfolio::Folium& folium )
{
    // this may call togather with handleSelectionChanged;
}

void
document::handleCheckStateChanged( adextension::iSessionManager *
                                   , const QString& file
                                   , const portfolio::Folium& folium
                                   , bool checked )
{
    ADDEBUG() << "## " << __FUNCTION__ << "\t" << file.toStdString()
              << folium.fullpath();
}

bool
document::find_all()
{
#if 0
    size_t idx(0);
    for ( auto sdmol: impl_->sdmols_ ) {
        if ( sdmol.mass() >= impl_->mass_range_.first && sdmol.mass() <= impl_->mass_range_.second ) {
            impl_->mols_[ sdmol.formula() ].emplace_back( std::move( sdmol ) );
        }
        if ( ( idx % 1000 ) == 0 )
            std::cerr << "\rloading mols: " << idx;
        idx++;
    }


    std::cerr << "\rtotal number of structures: " << idx << std::endl;
    std::cerr << "total number of formulae: " << impl_->mols_.size() << std::endl;

    std::cerr << "generating mass list..." << std::endl;
    impl_->mass_list_.clear();
    for ( auto mit = impl_->mols_.begin(); mit != impl_->mols_.end(); ++mit ) {
        if ( impl_->debug_mode_ ) {
            std::cout << "# mols:\t"
                      << std::distance( impl_->mols_.begin(), mit )
                      << "\t" << mit->first << "\t" << mit->second.size() << "\t" << mit->second.at( 0 ).mass();
            for ( auto& mol: mit->second ) {
                auto key = RDKit::MolToInchiKey( mol.sdmol()->mol() );
                std::cout << ",{" << key << "}";
            }
            std::cout << std::endl;
        }
        for ( auto ait = impl_->adducts_.begin(); ait != impl_->adducts_.end(); ++ait ) {
            auto formulae = adcontrols::ChemicalFormula::split( mit->first + " " + *ait );
            auto [mass, charge] = adcontrols::ChemicalFormula().getMonoIsotopicMass( formulae );
            impl_->mass_list_.emplace_back( mass, mit, ait );
        }
    }
    std::sort( impl_->mass_list_.begin(), impl_->mass_list_.end()
               , [](const auto& a, const auto& b){ return std::get<0>(a) < std::get<0>(b); } );

    ////////////////////////
    if ( impl_->debug_mode_ ) {
        std::cout << "## -- listing all masses" << std::endl;
        for ( const auto& a: impl_->mass_list_ ) {
            std::cout << "# adducts:"
                      << "\t" << std::setprecision( 5 ) << std::get<0>( a ) // mass
                       << "\t" << std::get<1>( a )->first // formula
                       << "\t" << *std::get<2>( a )       // adducts
                       << std::endl;
        }
        std::cout << std::endl; // add blank line
    }

    for ( auto mIt = impl_->ms_->begin(); mIt != impl_->ms_->end(); ++mIt ) {
        auto observed_mass = simple_mass_spectrum::mass( *mIt );
        auto lit = std::lower_bound( impl_->mass_list_.begin(), impl_->mass_list_.end()
                                     , observed_mass - impl_->mass_tolerance_ / 2.0
                                     , []( const auto& a, double b ){ return std::get<0>(a) < b; });
        std::vector< std::vector< impl::ion_mass_type >::const_iterator > candidates;
        if ( lit != impl_->mass_list_.end() ) {
            while ( std::get< 0 >(*lit) < (observed_mass + impl_->mass_tolerance_ / 2.0) )
                candidates.emplace_back( lit++ );
        }
        std::sort( candidates.begin(), candidates.end()
                   , [&](const auto& a, const auto& b){
                       return std::abs( observed_mass - std::get< 0 >( *a ) ) < std::abs( observed_mass - std::get< 0 >( *b ) );
                   });
        const size_t index = std::distance( impl_->ms_->begin(), mIt );
        for ( const auto& c: candidates ) {
            // isotope cluster
            auto cluster = isoCluster::compute( std::get< 1 >( *c )->first // formula
                                                , *std::get< 2 >( *c )     // adduct
                                                , 1.0e-4   // abundance low limit
                                                , 4000 );  // R.P.
            candidate t( std::get< 0 >( *c )         // exact mass
                         , std::get< 1 >( *c )->first  // formula := map[string, mol]
                         , *std::get< 2 >( *c )        // adduct
                         , std::get< 0 >( *c ) - simple_mass_spectrum::mass( (*impl_->ms_)[ index ] ) // mass error
                         , impl_->ms_->find_cluster( index, cluster )
                         , this->inchiKeys( std::get< 1 >( *c )->first )
                         , this->dataItems( std::get< 1 >( *c )->first )
                );
            impl_->ms_->add_a_candidate( index, std::move( t ) );
        }
    }
#endif
    return true;
}
