/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#pragma once

#include <adportable/debug.hpp>
#include <boost/exception/all.hpp>
#include <workaround/boost/archive/xml_woarchive.hpp>
#include <workaround/boost/archive/xml_wiarchive.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <adportable/portable_binary_iarchive.hpp>
#include <memory>
#include <iostream>

namespace adcontrols {

    //typedef boost::error_info< struct tag_errno, int > info;
    typedef boost::error_info< struct tag_errmsg, std::string > info;

    namespace internal {

        struct xmlSerializer {
            const char * name_;
            
            xmlSerializer( const char * name );

            template<class T> bool archive( std::wostream& os, const T& t ) {
                try {
                    boost::archive::xml_woarchive ar( os );
                    ar & boost::serialization::make_nvp( name_, t );
                    return true;
                } catch ( std::exception& ex ) {
                    BOOST_THROW_EXCEPTION( ex );// << info( "xml archive" );
                }
                return false;
            }

            template<class T> bool restore( std::wistream& is, T& t ) {
                try {
                    boost::archive::xml_wiarchive ar( is );
                    ar & boost::serialization::make_nvp( name_, t );
                    return true;
                } catch ( std::exception& ex ) {
                    BOOST_THROW_EXCEPTION( ex );// << error_info( "xml restore" );
                }
                return false;
            }
        };

        struct binSerializer {

            template<class T> bool archive( std::ostream& os, const T& t ) {
                try {
                    portable_binary_oarchive ar( os );
                    ar & t;
                    return true;
                } catch ( std::exception& ex ) {
                    BOOST_THROW_EXCEPTION( ex );// << error_info( "bin archive" );
                }
                return false;
            }

            template<class T> bool restore( std::istream& is, T& t ) {
                try {
                    portable_binary_iarchive ar( is );
                    ar & t;
                    return true;
                } catch ( std::exception& ex ) {
                    BOOST_THROW_EXCEPTION( ex );// << error_info( "bin restore" );
                }
                return false;
            }
        };

    }
}

