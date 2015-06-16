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
#include <adportable/uuid.hpp>
#include <adportable/date_string.hpp>
#include <adportable/profile.hpp>
#include <chrono>

using namespace adcontrols;

idAudit::idAudit() : uuid_( adportable::uuid()() )
                   , dateCreated_( adportable::date_string::logformat( std::chrono::system_clock::now(), true ) )
                   , idComputer_( adportable::profile::computer_name<wchar_t>() )
                   , idCreatedBy_( adportable::profile::user_login_id<wchar_t>() )
                   , nameCreatedBy_( adportable::profile::user_login_name<wchar_t>() )
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

const char *
idAudit::digest() const
{
    return digest_.c_str();
}

const char *
idAudit::dateCreated() const
{
    return dateCreated_.c_str();
}

const wchar_t *
idAudit::idComputer() const
{
    return idComputer_.c_str();
}

const wchar_t *
idAudit::idCreatedBy() const
{
    return idCreatedBy_.c_str();
}

const wchar_t *
idAudit::nameCreatedBy() const
{
    return nameCreatedBy_.c_str();
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
idAudit::setIdComputer( const wchar_t * value )
{
    idComputer_ = value ? value : L"";
}

void
idAudit::setIdCreatedBy( const wchar_t * value )
{
    idCreatedBy_ = value ? value : L"";
}

void
idAudit::setNameCreatedBy( const wchar_t * value )
{
    nameCreatedBy_ = value ? value : L"";
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
