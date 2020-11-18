/**************************************************************************
** Copyright (C) 2020 Toshinobu Hondo, Ph.D.
** Copyright (C) 2020 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
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

#include "configfile.hpp"
#include <boost/filesystem.hpp>
#include <adportable/profile.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/format.hpp>

using namespace aqmd3;

configFile::configFile()
    : inifile_( boost::filesystem::path( adportable::profile::user_config_dir<char>() ) / "QtPlatz" )
{
}

configFile::~configFile()
{
}

bool
configFile::saveResource( const std::string& res ) const
{
    if ( ! boost::filesystem::exists( inifile_ ) ) {
        boost::system::error_code ec;
        boost::filesystem::create_directories( inifile_, ec );
        if ( ec )
            return false; // error
    }
    if ( auto cres = loadResource() ) {
        if ( res == *cres )
            return true; // already exists; nothing to be done.
    }
    boost::property_tree::ptree pt;
    pt.put( "AQMD3.resource", res );
    boost::property_tree::write_ini( inifile_.string(), pt );
    return true;
}

boost::optional< std::string >
configFile::loadResource() const
{
    if ( boost::filesystem::exists( inifile_ ) ) {

        boost::property_tree::ptree pt;
        try {
            boost::property_tree::read_ini( inifile_.string(), pt );
        } catch ( std::exception& ex ) {
            return boost::none;
        }
        return pt.get_optional<std::string>( "AGMD3.resource" );
    }
    return boost::none;
}
