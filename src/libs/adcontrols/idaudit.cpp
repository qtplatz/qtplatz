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

# if defined _MSC_VER
#  pragma warning (disable:4996)
# endif

#include "idaudit.hpp"
#include "serializer.hpp"

#if defined _MSC_VER
#  pragma warning (default:4996)
#endif
#include <adportable/date_string.hpp>
#include <adportable/json/extract.hpp>
#include <adportable/json_helper.hpp>
#include <adportable/profile.hpp>
#include <adportable/utf.hpp>
#include <adportable/uuid.hpp>
#include <boost/json.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <chrono>
#include <locale>

using namespace adcontrols;
using adportable::utf;

idAudit::idAudit() : uuid_( adportable::uuid()() )
                   , dateCreated_( adportable::date_string::logformat( std::chrono::system_clock::now(), true ) )
                   , idComputer_( adportable::profile::computer_name<char>() )
                   , idCreatedBy_( adportable::profile::user_login_id<char>() )
                   , nameCreatedBy_( adportable::profile::user_login_name<char>() )
{
}

idAudit::idAudit( const idAudit& t ) : uuid_( t.uuid_ )
                                     , digest_( t.digest_ )
                                     , dateCreated_( t.dateCreated_ )
                                     , idComputer_( t.idComputer_ )
                                     , idCreatedBy_( t.idCreatedBy_ )
                                     , nameCreatedBy_( t.nameCreatedBy_ )
{
}

const std::string&
idAudit::digest() const
{
    return digest_;
}

const std::string&
idAudit::dateCreated() const
{
    return dateCreated_;
}

const std::string&
idAudit::idComputer() const
{
    return idComputer_;
}

const std::string&
idAudit::idCreatedBy() const
{
    return idCreatedBy_;
}

const std::string&
idAudit::nameCreatedBy() const
{
    return nameCreatedBy_;
}

const boost::uuids::uuid&
idAudit::uuid() const
{
    return uuid_;
}

void
idAudit::setUuid( const boost::uuids::uuid& id )
{
    uuid_ = id;
}

void
idAudit::setDigest( const char * digest )
{
    digest_ = digest ? digest : "";
}

void
idAudit::setDateCreated( const char * date )
{
    dateCreated_ = date ? date : "";
}

void
idAudit::setIdComputer( const char * value )
{
    idComputer_ = value ? value : "";
}

void
idAudit::setIdCreatedBy( const char * value )
{
    idCreatedBy_ = value ? value : "";
}

void
idAudit::setNameCreatedBy( const char * value )
{
    nameCreatedBy_ = value ? value : "";
}


//static
bool
idAudit::xml_archive( std::wostream& os, const idAudit& t )
{
    return internal::xmlSerializer("idAudit").archive( os, t );
}

//static
bool
idAudit::xml_restore( std::wistream& is, idAudit& t )
{
    return internal::xmlSerializer("idAudit").restore( is, t );
}

idAudit::operator boost::json::object () const
{
    return boost::json::value_from( *this ).as_object();
}

namespace adcontrols {

    void
    tag_invoke( boost::json::value_from_tag, boost::json::value& jv, const idAudit& t )
    {
        using adportable::utf;

        jv = boost::json::object{ { "idAudit"
            , {
                { "uuid", boost::uuids::to_string( t.uuid_ ) }
                , { "dateCreated",   t.dateCreated_ }
                , { "idComputer",    t.idComputer_  }
                , { "idCreatedBy",   t.idCreatedBy_  }
                , { "nameCreatedBy", t.nameCreatedBy_  }
                , { "digest",        t.digest_ }
            }
        }};
    }

    idAudit
    tag_invoke( boost::json::value_to_tag< idAudit >&, const boost::json::value& jv )
    {
        idAudit t;
        using namespace adportable::json;

        if ( jv.is_object() ) {
            auto obj = jv.as_object();
            extract( obj, t.uuid_, "uuid" );
            extract( obj, t.dateCreated_,   "dateCreated" );

            extract( obj, t.idComputer_,    "idComputer" );
            extract( obj, t.idCreatedBy_,   "idCreatedBy" );
            extract( obj, t.nameCreatedBy_, "nameCreatedBy" );
            extract( obj, t.digest_,        "digest" );
        }
        return t;
    }

}

namespace adcontrols {

    template< typename T = idAudit >
    class idAudit::archiver {
    public:
        template<class Archive>
        void serialize( Archive& ar, T& _, const unsigned int version ) {
            if ( version >= 1 ) {
                ar & BOOST_SERIALIZATION_NVP( _.uuid_ )
                    & BOOST_SERIALIZATION_NVP( _.digest_ )
                    & BOOST_SERIALIZATION_NVP( _.dateCreated_ )
                    & BOOST_SERIALIZATION_NVP( _.idComputer_ )
                    & BOOST_SERIALIZATION_NVP( _.idCreatedBy_ )
                    & BOOST_SERIALIZATION_NVP( _.nameCreatedBy_ )
                    ;
            } else if ( version == 0 ) {
                std::wstring idComputer, idCreatedBy, nameCreatedBy;
                ar & BOOST_SERIALIZATION_NVP( _.uuid_ )
                    & BOOST_SERIALIZATION_NVP( _.digest_ )
                    & BOOST_SERIALIZATION_NVP( _.dateCreated_ )
                    & BOOST_SERIALIZATION_NVP( idComputer )
                    & BOOST_SERIALIZATION_NVP( idCreatedBy )
                    & BOOST_SERIALIZATION_NVP( nameCreatedBy )
                    ;
                if ( Archive::is_loading::value ) {
                    _.idComputer_ = utf::to_utf8( idComputer );
                    _.idCreatedBy_ = utf::to_utf8( idCreatedBy );
                    _.nameCreatedBy_ = utf::to_utf8( nameCreatedBy );
                }
            }
        }
    };

    template<> ADCONTROLSSHARED_EXPORT void
    serialize( portable_binary_iarchive& ar, idAudit& t, const unsigned int version )
    {
#if __GNUC__
        idAudit::archiver<idAudit>().serialize( ar, t, version );
#else
        idAudit::archiver().serialize( ar, t, version );
#endif
    }

    template<> ADCONTROLSSHARED_EXPORT void
    serialize( portable_binary_oarchive& ar, idAudit& t, const unsigned int version )
    {
#if __GNUC__
        idAudit::archiver<idAudit>().serialize( ar, t, version );
#else
        idAudit::archiver().serialize( ar, t, version );
#endif
    }

    ///////// XML archive ////////
    template<> ADCONTROLSSHARED_EXPORT void
    serialize( boost::archive::xml_woarchive& ar, idAudit& t, const unsigned int version )
    {
        // saving
#if __GNUC__
        idAudit::archiver<idAudit>().serialize( ar, t, version );
#else
        idAudit::archiver().serialize( ar, t, version );
#endif
    }

    template<> ADCONTROLSSHARED_EXPORT void
    serialize( boost::archive::xml_wiarchive& ar, idAudit& t, const unsigned int version )
    {
#if __GNUC__
        idAudit::archiver<idAudit>().serialize( ar, t, version );
#else
        idAudit::archiver().serialize( ar, t, version );
#endif
    }

}
