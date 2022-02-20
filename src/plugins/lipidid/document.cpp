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

        std::shared_ptr< lipidid::simple_mass_spectrum >
        find_all( std::shared_ptr< const adcontrols::MassSpectrum >
                  , std::shared_ptr< adwidgets::ProgressInterface > ) const;

        std::unique_ptr< QSettings > settings_;
        QSqlDatabase db_;
        std::shared_ptr< adfs::sqlite > sqlite_;
        std::shared_ptr< const adcontrols::MassSpectrum > ms_;
        std::shared_ptr< const adcontrols::MassSpectrum > refms_;
        std::shared_ptr< lipidid::simple_mass_spectrum > simple_mass_spectrum_;
        adcontrols::MetIdMethod method_;
        std::map< std::string, double > logP_;
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
    impl_->method_ = std::move( t );
    if ( ! impl_->ms_ ) {
        ADDEBUG() << "no spectrum to be processed.";
        return false;
    }
    if ( impl_->ms_ ) {
        qtwrapper::waitCursor waitCursor;
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

void
document::setLogP( const std::string& InChIKey, double value )
{
    impl_->logP_[ InChIKey ] = value;
}

double
document::logP( const std::string& InChIKey ) const
{
    auto it = impl_->logP_.find( InChIKey );
    if ( it != impl_->logP_.end() )
        return it->second;

    adfs::stmt sql( *impl_->sqlite_ );
    sql.prepare( "SELECT SlogP FROM mols WHERE inchiKey like ?" );
    sql.bind( 0 ) = InChIKey;
    if ( sql.step() == adfs::sqlite_row ) {
        auto [ value ] = adfs::get_column_values< double >( sql );
        impl_->logP_[ InChIKey ] = value;
        return value;
    }
    return 0;
}
