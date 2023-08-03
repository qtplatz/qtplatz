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
#include "ionreaction.hpp"
#include "isocluster.hpp"
#include "isopeak.hpp"
#include "make_reference_spectrum.hpp"
#include "metidprocessor.hpp"
#include "sqlexport.hpp"
#include <coreplugin/progressmanager/progressmanager.h>
#include <qtwrapper/waitcursor.hpp>
#include <adcontrols/annotation.hpp>
#include <adcontrols/annotations.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/ionreactionmethod.hpp>
#include <adcontrols/isocluster.hpp>
#include <adcontrols/make_combination.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/metidmethod.hpp>
#include <adcontrols/molecule.hpp>
#include <adcontrols/targeting.hpp>
#include <adfs/get_column_values.hpp>
#include <adfs/sqlite.hpp>
#include <adportable/debug.hpp>
#include <adportable/float.hpp>
#include <adportable/json/extract.hpp>
#include <adportfolio/folder.hpp>
#include <adportfolio/folium.hpp>
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
#include <filesystem>
#include <fstream>
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

    class document::impl {
    public:
        ~impl() {
            // ADDEBUG() << "## lipidid::document::impl DTOR ##";
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

        std::shared_ptr< lipidid::simple_mass_spectrum >
        find_all( std::shared_ptr< const adcontrols::MassSpectrum >
                  , std::shared_ptr< adwidgets::ProgressInterface > ) const;

        std::unique_ptr< QSettings > settings_;
        QSqlDatabase db_;
        std::shared_ptr< adfs::sqlite > sqlite_;
        std::filesystem::path filename_;
        std::shared_ptr< const adcontrols::MassSpectrum > ms_;
        std::shared_ptr< const adcontrols::MassSpectrum > refms_;
        std::shared_ptr< const adcontrols::MassSpectrum > overlay_;
        std::shared_ptr< const adcontrols::MassSpectrum > matched_;
        std::shared_ptr< lipidid::simple_mass_spectrum > simple_mass_spectrum_;
        adcontrols::MetIdMethod method_;
        std::map< std::string, double > logP_;
        QString selectedFormula_;
    };
}

using lipidid::document;

document::~document()
{
    // ADDEBUG() << "## lipidid::document DTOR ##";
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

adfs::sqlite *
document::sqlite()
{
    return impl_->sqlite_.get();
}

void
document::handleAddProcessor( adextension::iSessionManager *, const QString& file )
{
    // ADDEBUG() << "## " << __FUNCTION__ << "\t" << file.toStdString();
}

// change node (folium) selection
void
document::handleSelectionChanged( adextension::iSessionManager *
                                  , const QString& file
                                  , const portfolio::Folium& folium )
{
    // ADDEBUG() << "## " << __FUNCTION__ << "\t" << file.toStdString();
    using portfolio::is_any_shared_of;
    if ( is_any_shared_of< adcontrols::MassSpectrum, const adcontrols::MassSpectrum >( folium ) ) {
        using portfolio::get_shared_of;
        if ( auto ptr = get_shared_of< const adcontrols::MassSpectrum, adcontrols::MassSpectrum >()( folium.data() ) ) {
            if ( ptr->isCentroid() ) {
                impl_->ms_ = ptr;
                impl_->filename_ = std::filesystem::path( file.toStdString() );
                emit dataChanged( folium );
                load_all();
            }
        }
    }
}

// data contents changed
void
document::handleProcessed( adextension::iSessionManager *
                           , const QString& file, const portfolio::Folium& folium )
{
    ADDEBUG() << "## " << __FUNCTION__ << "\t" << file.toStdString();
    // this may call togather with handleSelectionChanged;
}

void
document::handleCheckStateChanged( adextension::iSessionManager *
                                   , const QString& file
                                   , const portfolio::Folium& folium
                                   , bool checked )
{
    // ADDEBUG() << "## " << __FUNCTION__ << "\t" << file.toStdString()
    //           << folium.fullpath();
}

void
document::handleCheckState( int index, double mass, bool checked )
{
    if ( auto self = impl_->simple_mass_spectrum_ ) {
        auto& value = self->at( index );
        if ( adportable::compare< double >::approximatelyEqual( mass_value_t::mass( value ), mass ) ) {
            mass_value_t::checked( value ) = checked;
        }
    }
}

void
document::handleFormulaSelected( const QString& formula, double abundance, int index )
{
    // ADDEBUG() << "## " << __FUNCTION__ << "\t" << formula.toStdString() << ", " << abundance << ", index: " << index;
    try {
        if ( auto self = impl_->simple_mass_spectrum_ ) {
            auto candidates = self->candidates( index );
            auto it = std::find_if( candidates.begin()
                                    , candidates.end()
                                    , [&](const auto& c){ return ( c.formula() + c.adduct() ) == formula.toStdString(); });
            if ( it != candidates.end() ) {
                if ( auto ms = reference_mass_spectrum() ) {
                    if ( auto matched = self->make_spectrum( *it, ms ) ) {
                        impl_->matched_ = matched;
                        emit onMatchedSelected( index );
                    }
                }
            }
        }
    } catch ( std::out_of_range& ex ) {
        ADDEBUG() << "## Exception: " << ex.what();
    } catch ( std::exception& ex ) {
        ADDEBUG() << "## Exception: " << ex.what();
    }

    try {
        if ( impl_->selectedFormula_ != formula ) {
            impl_->selectedFormula_ = formula;
            auto metid = lipidid::MetIdProcessor::create( impl_->method_ );
            if ( auto ms = reference_mass_spectrum() ) {
                impl_->overlay_ = metid->compute_reference_spectrum( formula.toStdString(), abundance, ms );
                emit onFormulaSelected( impl_->selectedFormula_, abundance );
            }
        }
    } catch ( std::exception& ex ) {
        ADDEBUG() << "exception: " << ex.what();
    }
}


std::shared_ptr< const adcontrols::MassSpectrum >
document::reference_mass_spectrum() const
{
    return impl_->refms_;
}

std::shared_ptr< const adcontrols::MassSpectrum >
document::overlay_mass_spectrum() const
{
    return impl_->overlay_;
}

std::shared_ptr< const adcontrols::MassSpectrum >
document::matched_mass_spectrum() const
{
    return impl_->matched_;
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
document::export_ion_reactions( adcontrols::IonReactionMethod&& t, bool testing )
{
    // ADDEBUG() << boost::json::value_from( t );
    ADDEBUG() << "===== Ionization: " << t.i8n() << "\t" << t.description() << " testing: " << testing;

    std::filesystem::path path( qtwrapper::settings( *impl_->settings_ ).recentFile( "LIPID_MAPS", "Files" ).toStdString() );
    auto nfile = ( path.parent_path() / ( path.stem().string() + "_rxn" ) ).replace_extension( ".db" );

    SQLExport e;
    if ( e.create_database( nfile ) ) {
        e.export_ion_reactions( std::move( t ), testing );
    }
    return true;
}

bool
document::find_all( adcontrols::MetIdMethod&& t )
{
    impl_->method_ = std::move( t );
    // auto json = boost::json::serialize( boost::json::object{{ "metIdMethod", impl_->method_ }} );
    // impl_->settings_->setValue( QString(Constants::THIS_GROUP) + "/MetIdMethod", QByteArray( json.data(), json.size() ) );
    // ADDEBUG() << json;

    if ( ! impl_->ms_ ) {
        ADDEBUG() << "no spectrum to be processed.";
        return false;
    }
    if ( impl_->ms_ ) {
        qtwrapper::waitCursor waitCursor;
        auto p = std::make_shared< adwidgets::ProgressInterface >();
        Core::ProgressManager::addTask( p->progress.future(), "Processing...", Constants::LIPIDID_TASK_FIND_ALL );

        auto metid = lipidid::MetIdProcessor::create( impl_->method_ );
        auto future = std::async( std::launch::async, [=,this](){ return metid->find_all( *impl_->sqlite_, impl_->ms_, p );} );

        while ( std::future_status::ready != future.wait_for( std::chrono::milliseconds( 100 ) ) )
            QCoreApplication::instance()->processEvents();
        future.wait();

        std::tie( impl_->ms_, impl_->refms_, impl_->simple_mass_spectrum_ ) = future.get();

        emit idCompleted();
    }
    return true;
}

std::optional< std::string >
document::find_svg( const std::string& InChIKey ) const
{
    if ( auto svg = lipidid::moldb::instance().svg( InChIKey ) )
        return svg;
    adfs::stmt sql( *impl_->sqlite_ );
    sql.prepare( "SELECT svg FROM mols WHERE inchiKey = ?" );
    sql.bind( 1 ) = InChIKey;
    if ( sql.step() != adfs::sqlite_row ) {
        ADDEBUG() << "sql.error: " << sql.errmsg();
        return {};
    }
    auto [ svg ] = adfs::get_column_values< std::string >( sql );
    lipidid::moldb::instance().addSVG( InChIKey, std::move( svg ) );
    return lipidid::moldb::instance().svg( InChIKey );
}

std::shared_ptr< lipidid::mol >
document::find_mol( const std::string& InChIKey ) const
{
    adfs::stmt sql( *impl_->sqlite_ );
    sql.prepare( "SELECT id,formula,smiles,SlogP FROM mols WHERE inchiKey = ?" );
    sql.bind( 1 ) = InChIKey;
    while ( sql.step() == adfs::sqlite_row ) {
        auto [ id, formula, smiles, SlogP ] = adfs::get_column_values< int64_t, std::string, std::string, double >( sql );
        auto mol = std::make_shared< lipidid::mol >( std::make_tuple( id, formula, smiles, InChIKey, SlogP ) );
        return mol;
    }
    return {};
}

std::filesystem::path
document::dataFilename() const
{
    return impl_->filename_;
}

void
document::save_all() const
{
    if ( impl_->simple_mass_spectrum_ && std::filesystem::exists( impl_->filename_ ) ) {
        auto file( impl_->filename_ );
        file.replace_extension( ".json" );
        const auto& method = impl_->simple_mass_spectrum_->method();
        if ( method ) {
            std::ofstream( file )
                << boost::json::object{{ "metid_method", *method }
                    , { "simple_mass_spectrum", *impl_->simple_mass_spectrum_ }  };
        } else {
            std::ofstream( file )
                << boost::json::object{{ "simple_mass_spectrum", *impl_->simple_mass_spectrum_ }};
        }
    }
}

void
document::load_all() const
{
    auto file( impl_->filename_ );
    file.replace_extension( ".json" );
    if ( std::filesystem::exists( file ) ) {
        std::ifstream is( file );
        boost::json::stream_parser p;
        std::string line;
        boost::system::error_code ec;
        while( std::getline( is, line ) )    {
            p.write( line, ec );
            if ( ec )
                return;
        }
        p.finish( ec );
        if ( ec )
            return;
        auto jv = p.release();

        std::unique_ptr< adcontrols::MetIdMethod > method;
        if ( auto top = jv.as_object().if_contains( "metid_method" ) ) {
            method = std::make_unique< adcontrols::MetIdMethod >( boost::json::value_to< adcontrols::MetIdMethod >( *top ) );
            impl_->method_ = *method;
            emit metIdMethodChanged( impl_->method_ );
        }

        if ( auto top = jv.as_object().if_contains( "simple_mass_spectrum" ) ) {
            auto data = std::make_shared< lipidid::simple_mass_spectrum >();
            try {
                *data = boost::json::value_to< lipidid::simple_mass_spectrum >( *top );
                data->set_method( std::move( method ) );
                if ( auto refms = make_reference_spectrum()( *impl_->ms_, *data ) ) {
                    impl_->simple_mass_spectrum_ = std::move( data );
                    impl_->refms_ = std::move( refms );
                }
                emit idCompleted();
            } catch ( std::exception& ex ) {
                ADDEBUG() << "### exception: json::value_to " << ex.what();
            }
        }
    }
}
