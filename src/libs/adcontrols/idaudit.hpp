/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef IDAUDIT_HPP
#define IDAUDIT_HPP

#include "adcontrols_global.h"

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_serialize.hpp>
#include <boost/json/fwd.hpp>
#include <boost/json/value_to.hpp>
#include <string>

namespace boost {  namespace json { class object; } }

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT idAudit;

    template< typename Archive > void serialize(Archive & ar, idAudit&, const unsigned int version);

    class idAudit {
    public:
        idAudit();
        idAudit( const idAudit& );
        const std::string& digest() const;
        const std::string& dateCreated() const;
        const std::string& idComputer() const;
        const std::string& idCreatedBy() const;
        const std::string& nameCreatedBy() const;
        const boost::uuids::uuid& uuid() const;

        void setUuid( const boost::uuids::uuid& id );
        void setDigest( const char * );
        void setDateCreated( const char * );
        void setIdComputer( const char * );
        void setIdCreatedBy( const char * );
        void setNameCreatedBy( const char * );

        static bool xml_archive( std::wostream&, const idAudit& );
        static bool xml_restore( std::wistream&, idAudit& );

        operator boost::json::object () const;

    private:
        boost::uuids::uuid uuid_;
        std::string digest_;
        std::string dateCreated_;

        std::string idComputer_;
        std::string idCreatedBy_;
        std::string nameCreatedBy_;

        template< typename T > class archiver;
        friend class archiver< idAudit >;
        friend class archiver< const idAudit >;
        template< typename Archive > friend void serialize(Archive & ar, idAudit&, const unsigned int version);

        friend ADCONTROLSSHARED_EXPORT void tag_invoke( boost::json::value_from_tag, boost::json::value&, const idAudit& );
        friend ADCONTROLSSHARED_EXPORT idAudit tag_invoke( boost::json::value_to_tag< idAudit >&, const boost::json::value& jv );
    };

    ADCONTROLSSHARED_EXPORT
    void tag_invoke( boost::json::value_from_tag, boost::json::value&, const idAudit& );

    ADCONTROLSSHARED_EXPORT
    idAudit tag_invoke( boost::json::value_to_tag< idAudit >&, const boost::json::value& jv );

}

BOOST_CLASS_VERSION( adcontrols::idAudit, 1 )

#endif // IDAUDIT_HPP
