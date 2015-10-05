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

#include "identify.hpp"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>
#include <adportable/portable_binary_iarchive.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>

namespace acqrscontrols {
    namespace u5303a {

        template<typename T = identify>
        class identify_archive {
        public:
            template<class Archive>
            void serialize( Archive& ar, T& _, const unsigned int version ) {
                using namespace boost::serialization;
                ar & BOOST_SERIALIZATION_NVP( _.Identifier_ );
                ar & BOOST_SERIALIZATION_NVP( _.Revision_ );
                ar & BOOST_SERIALIZATION_NVP( _.Vendor_ );
                ar & BOOST_SERIALIZATION_NVP( _.Description_ );
                ar & BOOST_SERIALIZATION_NVP( _.InstrumentModel_ );
                ar & BOOST_SERIALIZATION_NVP( _.FirmwareRevision_ );
                ar & BOOST_SERIALIZATION_NVP( _.SerialNumber_ );
                ar & BOOST_SERIALIZATION_NVP( _.Options_ );
                ar & BOOST_SERIALIZATION_NVP( _.IOVersion_ );
            }
        };

        template<> void ACQRSCONTROLSSHARED_EXPORT
        identify::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
        {
            identify_archive<>().serialize( ar, *this, version );
        }

        template<> ACQRSCONTROLSSHARED_EXPORT
        void identify::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
        {
            identify_archive<>().serialize( ar, *this, version );
        }

        template<> ACQRSCONTROLSSHARED_EXPORT
        void identify::serialize( portable_binary_oarchive& ar, const unsigned int version )
        {
            identify_archive<>().serialize( ar, *this, version );
        }

        template<> ACQRSCONTROLSSHARED_EXPORT
        void identify::serialize( portable_binary_iarchive& ar, const unsigned int version )
        {
            identify_archive<>().serialize( ar, *this, version );
        }
    }
}

using namespace acqrscontrols::u5303a;

identify::identify() : NbrADCBits_( 12 )
{
}

identify::identify( const identify& t ) : Identifier_( t.Identifier_ )
                                        , Revision_( t.Revision_ )
                                        , Vendor_( t.Vendor_ )
                                        , Description_( t.Description_ )
                                        , InstrumentModel_( t.InstrumentModel_ )
                                        , FirmwareRevision_( t.FirmwareRevision_ )
                                        , SerialNumber_( t.SerialNumber_ )
                                        , Options_( t.Options_ )
                                        , IOVersion_( t.IOVersion_ )
                                        , NbrADCBits_( t.NbrADCBits_ )
{
}

std::string&
identify::Identifier()
{
    return Identifier_;
}

std::string&
identify::Revision()
{
    return Revision_;
}

std::string&
identify::Vendor()
{
    return Vendor_;
}

std::string&
identify::Description()
{
    return Description_;
}

std::string&
identify::InstrumentModel()
{
    return InstrumentModel_;
}

std::string&
identify::FirmwareRevision()
{
    return FirmwareRevision_;
}

std::string&
identify::SerialNumber()
{
    return SerialNumber_;
}

std::string&
identify::Options()
{
    return Options_;
}

std::string&
identify::IOVersion()
{
    return IOVersion_;
}

uint32_t&
identify::NbrADCBits()
{
    return NbrADCBits_;
}

const std::string&
identify::Identifier() const
{
    return Identifier_;
}

const std::string&
identify::Revision() const
{
    return Revision_;
}

const std::string&
identify::Vendor() const
{
    return Vendor_;
}

const std::string&
identify::Description() const
{
    return Description_;
}

const std::string&
identify::InstrumentModel() const
{
    return InstrumentModel_;
}

const std::string&
identify::FirmwareRevision() const
{
    return FirmwareRevision_;
}

const std::string&
identify::SerialNumber() const
{
    return SerialNumber_;
}

const std::string&
identify::Options() const
{
    return Options_;
}

const std::string&
identify::IOVersion() const
{
    return IOVersion_;
}

uint32_t
identify::NbrADCBits() const
{
    return NbrADCBits_;
}

