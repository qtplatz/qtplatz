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

#include "idaudit.hpp"
#include <adportable/uuid.hpp>
#include <adportable/date_string.hpp>
#include <adportable/profile.hpp>
#include <chrono>

using namespace adcontrols;

idAudit::idAudit() : uuid_( adportable::uuid()() )
                   , idCreatedBy_( adportable::profile::user_login_name<wchar_t>() )
                   , nameCreatedBy_( idCreatedBy_ )
                   , dateCreated_( adportable::date_string::logformat( std::chrono::system_clock::now() ) )
{
}

idAudit::idAudit( const idAudit& t ) : uuid_( t.uuid_ )
                                     , digest_( t.digest_ )
                                     , dateCreated_( t.dateCreated_ )
                                     , idTerminal_( t.idTerminal_ )
                                     , idCreatedBy_( t.idCreatedBy_ )
                                     , nameCreatedBy_( t.nameCreatedBy_ )
{
}

