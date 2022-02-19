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
#include "mol.hpp"
#include "simple_mass_spectrum.hpp"
#include "candidate.hpp"
#include "constants.hpp"
#include "isocluster.hpp"
#include "isopeak.hpp"
#include "metidprocessor.hpp"
#include <coreplugin/progressmanager/progressmanager.h>
#include <qtwrapper/waitcursor.hpp>
#include <adcontrols/annotation.hpp>
#include <adcontrols/annotations.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/metidmethod.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adportable/debug.hpp>
#include <adportfolio/folium.hpp>
#include <adportfolio/folder.hpp>
#include <adfs/sqlite.hpp>
#include <adfs/get_column_values.hpp>
#include <adwidgets/progressinterface.hpp>
#include <qtwrapper/settings.hpp>
#include <app/app_version.h> // <-- for Core::Constants::IDE_SETTINGSVARIANT_STR
#include <QCoreApplication>
#include <QMessageBox>
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <boost/filesystem.hpp>
#include <boost/json.hpp>
#include <tuple>
#include <future>
#include <mutex>

Q_DECLARE_METATYPE( portfolio::Folium )

namespace lipidid {

    struct user_preference {
        static boost::filesystem::path path( QSettings * settings ) {
            boost::filesystem::path dir( settings->fileName().toStdWString() );
            return dir.remove_filename() / "lipidid";
        }
    };


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

        void handleConnectionChanged( const std::string& fpath ) {
            if ( auto sqlite = std::make_shared< adfs::sqlite >() ) {
                if ( sqlite->open( fpath.c_str(), adfs::readonly ) ) {
                    sqlite_ = std::move( sqlite );
                }
            }
        }

        std::vector< std::string >
        getInChIKeys( const std::string& formula ) const {
            std::vector< std::string > t;
            auto it = mols_.find( formula );
            if ( it != mols_.end() ) {
                for ( const auto& mol: it->second )
                    t.emplace_back( mol.inchikey() );
            }
            return t;
        }

        std::shared_ptr< lipidid::simple_mass_spectrum > find_all( std::shared_ptr< const adcontrols::MassSpectrum >
                                                                   , std::shared_ptr< adwidgets::ProgressInterface > ) const;

        std::unique_ptr< QSettings > settings_;
        QSqlDatabase db_;
        std::shared_ptr< adfs::sqlite > sqlite_;
        std::shared_ptr< const adcontrols::MassSpectrum > ms_;
        std::shared_ptr< const adcontrols::MassSpectrum > refms_;
        std::shared_ptr< lipidid::simple_mass_spectrum > simple_mass_spectrum_;
        adcontrols::MetIdMethod method_;
        std::map< std::string, std::vector< lipidid::mol > > mols_; // stdformula, vector< mol >
        std::vector< reference_mass > reference_list_;
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
                impl_->handleConnectionChanged( fpath.string() );
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

std::shared_ptr< const adcontrols::MassSpectrum >
document::reference_mass_spectrum() const
{
    return impl_->refms_;
}

std::tuple< std::shared_ptr< const adcontrols::MassSpectrum > // acquired spectrum
            , std::shared_ptr< const adcontrols::MassSpectrum > // reference (calculated) spectrum
            , std::shared_ptr< const lipidid::simple_mass_spectrum > // reference (calculated) spectrum
            >
document::getResultSet() const
{
    return { impl_->ms_, impl_->refms_, impl_->simple_mass_spectrum_ };
}

bool
document::find_all( adcontrols::MetIdMethod&& t )
{
    qtwrapper::waitCursor waitCursor;

    impl_->reference_list_.clear();
    impl_->method_ = std::move( t );
    if ( ! impl_->ms_ ) {
        ADDEBUG() << "no spectrum to be processed.";
        return false;
    }
    if ( impl_->ms_ ) {
        auto p = std::make_shared< adwidgets::ProgressInterface >();
        Core::ProgressManager::addTask( p->progress.future(), "Processing...", Constants::LIPIDID_TASK_FIND_ALL );

        auto metid = lipidid::MetIdProcessor::create( impl_->method_ );
        auto future = std::async( std::launch::async, [=](){ return metid->find_all( *impl_->sqlite_, impl_->ms_, p );} );

        while ( std::future_status::ready != future.wait_for( std::chrono::milliseconds( 100 ) ) )
            QCoreApplication::instance()->processEvents();
        future.wait();

        std::tie( impl_->ms_, impl_->refms_, impl_->simple_mass_spectrum_ ) = future.get();

        emit idCompleted();
    }

    return true;
}

#if 0
std::shared_ptr< lipidid::simple_mass_spectrum >
document::impl::find_all( std::shared_ptr< const adcontrols::MassSpectrum > ms
                          , std::shared_ptr< adwidgets::ProgressInterface > progress ) const
{
    double mass_tolerance = method_.tolerance( method_.toleranceMethod() );
    ADDEBUG() << "mass tolerance: " << mass_tolerance * 1000 << "mDa";

    using lipidid::simple_mass_spectrum;
    using lipidid::mass_value_t;
    auto tms = std::make_shared< lipidid::simple_mass_spectrum >();
    tms->populate( *ms, [](auto value){ return mass_value_t::color( value ) == 15; });
    if ( tms->size() == 0 ) {
        ADDEBUG() << "no colored peak.";
        return {};
    }

    (*progress)( 0, tms->size() );

    ADDEBUG() << "populating mols from database...";
    size_t counts(0);
    if ( impl_->sqlite_ ) {
        adfs::stmt sql( *impl_->sqlite_ );
        sql.prepare( "SELECT id,formula,smiles,inchiKey FROM mols WHERE mass < 1200 ORDER BY mass" );
        while ( sql.step() == adfs::sqlite_row ) {
            ++counts;
            auto [ id, formula, smiles, inchikey ] = adfs::get_column_values< int64_t, std::string, std::string, std::string >( sql );
            impl_->mols_[ formula ].emplace_back( std::make_tuple( id, formula, smiles, inchikey ) );
        }
    }

    (*progress)( 0, tms->size() );

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
}
#endif
