/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include "acqiris_method.hpp"
#include "acqiriswidget.hpp"
#include "digitizer.hpp"
#include "task.hpp"
#include "tcp_server.hpp"
#include "waveform.hpp"
#include <QSettings>
#include <boost/serialization/nvp.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <boost/filesystem/path.hpp>
#include <fstream>
#include <iostream>

document *
document::instance()
{
    static document __instance;
    return &__instance;
}

class digitizer *
document::digitizer()
{
    static class digitizer __acqiris;
    return &__acqiris;
}

document::document( QObject * parent ) : QObject( parent )
                                       , method_( std::make_shared< aqdrv4::acqiris_method >() )
                                       , settings_( std::make_unique< QSettings >( QSettings::IniFormat
                                                                                   , QSettings::UserScope
                                                                                   , QLatin1String( "acqiris" )
                                                                                   , QLatin1String( "acqiris" )
                                                        ) )
{
}

document::~document()
{
}

bool
document::initialSetup()
{
    boost::filesystem::path path ( settings_->fileName().toStdString() );

    auto name = path.parent_path() / "acqiris_method.xml";

    if ( auto m = load( name.string() ) )
         set_acqiris_method( m );
    
    return true;
}

bool
document::finalClose()
{
    task::instance()->finalize();

    boost::filesystem::path path ( settings_->fileName().toStdString() );
    auto name = path.parent_path() / "acqiris_method.xml";

    settings_->setValue( "DefaultMethod", QString::fromStdString( name.string() ) );
    
    save( name.string(), method_ );
    
    return true;
}

QSettings *
document::settings()
{
    return nullptr;
}

bool
document::digitizer_initialize()
{
    task::instance()->initialize();
    
    auto aqrs = digitizer();
    
    if ( aqrs->initialize() ) {
        if ( aqrs->findDevice() ) {
            task::instance()->prepare_for_run( aqrs, document::instance()->acqiris_method() );
        }
    }
}

void
document::push( std::shared_ptr< waveform >&& d )
{
    que_.emplace_back( d );
    if ( que_.size() >= 4096 )
        que_.erase( que_.begin(), que_.begin() + 2048  );
}

std::shared_ptr< waveform >
document::recentWaveform()
{
    if ( !que_.empty() )
        return que_.back();
    return nullptr;
}

std::shared_ptr< const aqdrv4::acqiris_method >
document::acqiris_method()
{
    std::lock_guard< std::mutex > lock( mutex_ );
    return method_;
}

void
document::set_acqiris_method( std::shared_ptr< aqdrv4::acqiris_method > p )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    method_ = p;
}

void
document::set_server( std::unique_ptr< aqdrv4::server::tcp_server >&& server )
{
    server_ = std::move( server );
}

bool
document::save( const std::string& file, std::shared_ptr< const aqdrv4::acqiris_method > p )
{
    std::wofstream of( file );
    try {
        boost::archive::xml_woarchive ar( of );
        ar & boost::serialization::make_nvp("aqdrv4", *p );
    } catch ( ... ) {
        return false;
    }
    return true;
}

std::shared_ptr< aqdrv4::acqiris_method >
document::load( const std::string& file )
{
    auto p = std::make_shared< aqdrv4::acqiris_method >();
    std::wifstream of( file );
    try {
        boost::archive::xml_wiarchive ar( of );
        ar & boost::serialization::make_nvp("aqdrv4", *p );
        return p;
    } catch ( ... ) {
        return nullptr;
    }
}

void
document::handleValueChanged( std::shared_ptr< aqdrv4::acqiris_method > m, aqdrv4::SubMethodType subType )
{
    set_acqiris_method( m );
    task::instance()->prepare_for_run( digitizer(), document::instance()->acqiris_method() );
}
