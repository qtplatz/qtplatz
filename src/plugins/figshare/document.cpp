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
#include <adcontrols/figsharerest.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/sqlite.hpp>
#include <adprot/aminoacid.hpp>
#include <adportable/debug.hpp>
#include <adportable/json_helper.hpp>
#include <pug/http_client_async.hpp>
#include <app/app_version.h>
#include <qtwrapper/settings.hpp>
#include <QApplication>
#include <QTextEdit>
#include <QSqlError>
#include <QSqlQuery>
#include <boost/json.hpp>
#include <boost/format.hpp>
#include <future>
#include <sstream>

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

namespace figshare {

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
                                , QLatin1String( "REST" ) ) )
               , downloadCount_( 0 ) {
        }

        std::unique_ptr< QSettings > settings_;
        size_t downloadCount_;

        static QSettings * settings() { return impl::instance().settings_.get(); }

        static impl& instance() {
            static impl __impl;
            return __impl;
        }
    };
}


using namespace figshare;

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
document::JSTREST( const QByteArray& ba )
{
    auto rest= boost::json::value_to< adcontrols::JSTREST >(  adportable::json_helper::parse( ba.toStdString() ) );

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
    // emit pugReply( QByteArray( res.body().data() ), QString::fromStdString( url ) );
    ADDEBUG() << url;
}

void
document::figshareREST( const QByteArray& ba )
{
    auto rest= boost::json::value_to< adcontrols::figshareREST >(  adportable::json_helper::parse( ba.toStdString() ) );
    ADDEBUG() << boost::json::value_from( rest );

    rest.set_target( rest.articles_search() );
    ADDEBUG() << "######: " << rest.urlx();

    ///////////// conter clear //////////////
    impl::instance().downloadCount_ = 0;

    const int version = 11; // 1.0

    const auto& [port, host, body] = rest.urlx();

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
    emit figshareReply( QByteArray( res.body().data() ), QString::fromStdString( rest.url() ) );

    // find doi from resource_doi
    {
        std::vector< std::pair< std::string, int64_t > > idv;
        boost::system::error_code ec;
        auto value = boost::json::parse( res.body().data() );
        if ( value.is_array() ) {
            auto ja = value.as_array();
            for ( size_t i = 0; i < ja.size(); ++i ) {
                if ( auto doi = ja[i].as_object().if_contains( "doi" ) ) {
                    if ( auto id = ja[i].as_object().if_contains( "id" ) ) {
                        idv.emplace_back( doi->as_string(), id->as_int64() );
                    }
                }
            }
        }

        for ( auto& id: idv ) {
            auto target = rest.list_article_files( std::get<1>( id ) );
            figshare_rest( rest, target );
        }
    }

}

void
document::figshare_rest( const adcontrols::figshareREST& rest, const std::string& target )
{
    const int version = 11; // 1.1

    boost::asio::io_context ioc;
    boost::asio::ssl::context ctx{ boost::asio::ssl::context::tlsv12_client };
    {
        ctx.set_verify_mode(boost::asio::ssl::verify_peer | boost::asio::ssl::context::verify_fail_if_no_peer_cert);
        ctx.set_default_verify_paths();
        boost::certify::enable_native_https_server_verification(ctx);
    }

    const auto& [port, host, body] = rest.urlx();

    auto future = std::make_shared< session >( boost::asio::make_strand(ioc),  ctx )->run( host, port, target, version );
    ioc.run();

    auto res = future.get();
    emit figshareReply( QByteArray( res.body().data() ), QString::fromStdString( target ) );

    qApp->processEvents();

    //-------->
    boost::system::error_code ec;
    auto value = boost::json::parse( res.body().data() );
    if ( value.is_array() ) {
        auto ja = value.as_array();
        for ( size_t i = 0; i < ja.size(); ++i ) {
            if ( auto download_url = ja[i].as_object().if_contains( "download_url" ) ) {
                figshare_download( rest, ja[i], download_url->as_string().c_str() );
            }
        }
    }
}

void
document::figshare_download( const adcontrols::figshareREST& rest, const boost::json::value& value, const std::string& download_url )
{
    const int version = 11; // 1.1

    emit onDownloading();

    boost::asio::io_context ioc;
    boost::asio::ssl::context ctx{ boost::asio::ssl::context::tlsv12_client };
    {
        ctx.set_verify_mode(boost::asio::ssl::verify_peer | boost::asio::ssl::context::verify_fail_if_no_peer_cert);
        ctx.set_default_verify_paths();
        boost::certify::enable_native_https_server_verification(ctx);
    }

    ADDEBUG() << "download: " << adcontrols::figshareREST::parse_url( download_url );

    const auto& [port, host, body] = adcontrols::figshareREST::parse_url( download_url );

    auto future = std::make_shared< session >( boost::asio::make_strand(ioc),  ctx )->run( host, port, body, version );
    ioc.run();

    auto res = future.get();

    std::ostringstream o;

    if ( res.result_int() == 302 ) {

        for ( auto name: { boost::beast::http::field::location
                           , boost::beast::http::field::set_cookie
                           , boost::beast::http::field::content_type} ) {
            auto it = res.find( name );
            if ( it != res.end() ) {
                o << "name: " << it->name() << "\t" << it->value() << std::endl;
            }
        }
        o << "===================================" << std::endl;

        emit downloadReply( QByteArray( o.str().data() ), QString::fromStdString( download_url ) );
        qApp->processEvents();

        figshare_download( res );
    }
}


void
document::figshare_download( const boost::beast::http::response< boost::beast::http::string_body >& resp )
{
    const int version = 11; // 1.1

    auto itCookie = resp.find( boost::beast::http::field::set_cookie );
    if ( itCookie != resp.end() ) {
        auto it = resp.find( boost::beast::http::field::location );
        const auto& [port, host, target] = adcontrols::figshareREST::parse_url( it->value() );

        boost::asio::ssl::context ctx{ boost::asio::ssl::context::tlsv12_client };
        {
            ctx.set_verify_mode(boost::asio::ssl::verify_peer | boost::asio::ssl::context::verify_fail_if_no_peer_cert);
            ctx.set_default_verify_paths();
            boost::certify::enable_native_https_server_verification(ctx);
        }

        boost::beast::http::request< boost::beast::http::empty_body > req;
        req.version(  version  );
        req.method(  boost::beast::http::verb::get );
        req.target(  target  );
        req.set(  boost::beast::http::field::host, host );
        req.set(  boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING );
        req.set( boost::beast::http::field::cookie, itCookie->value() );

        boost::asio::io_context ioc;
        auto future = std::make_shared< session >( boost::asio::make_strand(ioc),  ctx )->run( host, port, req );
        ioc.run();

        auto res = future.get();

        {
            auto it = res.find( boost::beast::http::field::content_type );
            if ( it->value() == "text/plain" || it->value() == "text/csv" ) {
                // ADDEBUG() << res;
                auto server = res.find( boost::beast::http::field::server );
                if ( server != res.end() ) {
                    emit csvReply( QByteArray( res.body().data() ), QString::fromStdString( server->value() ), impl::instance().downloadCount_++ );
                    qApp->processEvents();
                }
            }
        }
    }
}
