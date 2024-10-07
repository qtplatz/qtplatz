/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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
//#include "chemconnection.hpp"
//#include "chemquery.hpp"
//#include "chemschema.hpp"
//#include "chemspider.hpp"
#include <adchem/drawing.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/pugrest.hpp>
#include <adcontrols/jstrest.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/sqlite.hpp>
#include <adprot/aminoacid.hpp>
#include <adportable/debug.hpp>
#include <adportable/json_helper.hpp>
#include <pug/http_client_async.hpp>
#include <app/app_version.h>
#include <qtwrapper/settings.hpp>
#include <QTextEdit>
#include <QSqlError>
#include <QSqlQuery>
#include <boost/json.hpp>
#include <future>

#if defined _MSC_VER
# pragma warning(disable:4267) // size_t to unsigned int possible loss of data (x64 int on MSC is 32bit)
#endif

#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/RDKitBase.h>
#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/SmilesParse/SmilesWrite.h>
#include <GraphMol/Descriptors/MolDescriptors.h>
#include <GraphMol/FileParsers/MolSupplier.h>
#include <GraphMol/inchi.h>
#if defined _MSC_VER
# pragma warning(default:4267) // size_t to unsigned int possible loss of data (x64 int on MSC is 32bit)
#endif

#include <QFileInfo>
#include <QMessageBox>
#include <QSettings>
#include <QSqlDatabase>
#include <filesystem>

#if OPENSSL_FOUND
# include <boost/certify/extensions.hpp>
# include <boost/certify/https_verification.hpp>
#else
# include <pug/root_certificates.hpp>
#endif

namespace rest {

    struct user_preference {
        static std::filesystem::path path( QSettings * settings ) {
            std::filesystem::path dir( settings->fileName().toStdWString() );
            return dir.remove_filename() / "rest";
        }
    };

    struct document::impl {

        impl() : settings_( std::make_unique< QSettings >(
                                QSettings::IniFormat, QSettings::UserScope
                                , QLatin1String( Core::Constants::IDE_SETTINGSVARIANT_STR )
                                , QLatin1String( "REST" ) ) ) {
        }

        std::unique_ptr< QSettings > settings_;
        QSqlDatabase db_;

        static QSettings * settings() { return impl::instance().settings_.get(); }
        static QSqlDatabase sqlDatabase() { return impl::instance().db_; }

        static impl& instance() {
            static impl __impl;
            return __impl;
        }
    };
}


using namespace rest;

std::mutex document::mutex_;

document::document(QObject *parent) : QObject(parent)
{
}

document *
document::instance()
{
    static document __instance;
    return &__instance;
}

void
document::initialSetup()
{
    std::filesystem::path dir = user_preference::path( impl::settings() );

    if ( !std::filesystem::exists( dir ) ) {
        if ( !std::filesystem::create_directories( dir ) ) {
            QMessageBox::information( 0, "chemistry::document"
                                      , QString( "Work directory '%1' can not be created" ).arg( dir.string().c_str() ) );
        }
    }
}

void
document::finalClose()
{
    std::filesystem::path dir = user_preference::path( impl::settings() );
    if ( !std::filesystem::exists( dir ) ) {
        if ( !std::filesystem::create_directories( dir ) ) {
            QMessageBox::information( 0, "chemistry::document"
                                      , QString( "Work directory '%1' can not be created" ).arg( dir.string().c_str() ) );
            return;
        }
    }
}

QSettings *
document::settings()
{
    return impl::settings();
}

void
document::PubChemREST( const QByteArray& ba )
{
    ADDEBUG() << __FUNCTION__;
#if PUBCHEM_REST
    auto rest= boost::json::value_to< adcontrols::PUGREST >(  adportable::json_helper::parse( ba.toStdString() ) );

    auto url = rest.pug_url().empty() ? adcontrols::PUGREST::to_url( rest, true ) : rest.pug_url();

    auto urlx = adcontrols::PUGREST::parse_url( url );
    ADDEBUG() << "url=" << urlx;
    const int version = 10; // 1.0

    // "pubchem.ncbi.nlm.nih.gov";
    const auto& [port, host, body] = urlx;

    boost::asio::io_context ioc;
    boost::asio::ssl::context ctx{ boost::asio::ssl::context::tlsv12_client };
# if OPENSSL_FOUND
    // verify SSL context
    {
        ctx.set_verify_mode(boost::asio::ssl::verify_peer | boost::asio::ssl::context::verify_fail_if_no_peer_cert);
        ctx.set_default_verify_paths();
        boost::certify::enable_native_https_server_verification(ctx);
    }
# else
    load_root_certificates(ctx);
# endif
    auto future = std::make_shared< session >( boost::asio::make_strand(ioc),  ctx )->run( host, port, body, version );
    ioc.run();

    auto res = future.get();
    emit pugReply( QByteArray( res.body().data() ), QString::fromStdString( url ) );
#endif
}

void
document::JSTREST( const QByteArray& ba )
{
# if OPENSSL_FOUND
    auto rest= boost::json::value_to< adcontrols::JSTREST >(  adportable::json_helper::parse( ba.toStdString() ) );

    // auto url = rest.pug_url().empty() ? adcontrols::JSTREST::to_url( rest, true ) : rest.pug_url();
    auto url = adcontrols::JSTREST::to_url( rest );
    ADDEBUG() << "url=" << url;

    auto urlx = adcontrols::PUGREST::parse_url( url );
    const int version = 11; // 1.0

    // "pubchem.ncbi.nlm.nih.gov";
    const auto& [port, host, body] = urlx;

    boost::asio::io_context ioc;
    boost::asio::ssl::context ctx{ boost::asio::ssl::context::tlsv12_client };
    {
        ctx.set_verify_mode(boost::asio::ssl::verify_peer | boost::asio::ssl::context::verify_fail_if_no_peer_cert);
        ctx.set_default_verify_paths();
        boost::certify::enable_native_https_server_verification(ctx);
    }
    auto future = std::make_shared< session >( boost::asio::make_strand(ioc),  ctx )->run( host, port, body, version );
    ioc.run();

    auto res = future.get();
    emit pugReply( QByteArray( res.body().data() ), QString::fromStdString( url ) );
    ADDEBUG() << url;
#endif
}
