/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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

#include "timedevents.hpp"
#include "timedevent.hpp"
#include "serializer.hpp"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <string>
#include <map>
#include <vector>

namespace adcontrols {
    namespace ControlMethod {

        template< typename T = TimedEvents >
        class TimedEvents_archive {
        public:
            friend class boost::serialization::access;
            template<class Archive> void serialize( Archive& ar, T& _, const unsigned int version )
            {
                using namespace boost::serialization;

                ar & BOOST_SERIALIZATION_NVP( _.vec_ );
            }
        };

        ////////// PORTABLE BINARY ARCHIVE //////////
        template<> void
            TimedEvents::serialize( portable_binary_oarchive& ar, const unsigned int version )
        {
            TimedEvents_archive<>().serialize( ar, *this, version );
        }

        template<> void
            TimedEvents::serialize( portable_binary_iarchive& ar, const unsigned int version )
        {
            TimedEvents_archive<>().serialize( ar, *this, version );
        }

        ///////// XML archive ////////
        template<> void
            TimedEvents::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
        {
            TimedEvents_archive<>().serialize( ar, *this, version );
        }

        template<> void
            TimedEvents::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
        {
            TimedEvents_archive<>().serialize( ar, *this, version );
        }

    }
}

using namespace adcontrols::ControlMethod;

TimedEvents::~TimedEvents()
{
}

TimedEvents::TimedEvents()
{
}

TimedEvents::TimedEvents( const TimedEvents& t ) : vec_( t.vec_ )
{
}

size_t
TimedEvents::size() const
{
    return vec_.size();
}

void
TimedEvents::clear()
{
    vec_.clear();
}

TimedEvents&
TimedEvents::operator << ( const TimedEvent& t )
{
    vec_.push_back( t );
    return *this;
}

TimedEvents::iterator
TimedEvents::begin()
{
    return vec_.begin();
}

TimedEvents::iterator
TimedEvents::end()
{
    return vec_.end();
}

TimedEvents::const_iterator
TimedEvents::begin() const
{
    return vec_.begin();
}

TimedEvents::const_iterator
TimedEvents::end() const
{
    return vec_.end();
}

bool
TimedEvents::archive( std::ostream& os, const TimedEvents& t )
{
    return internal::binSerializer().archive( os, t );
}

bool
TimedEvents::restore( std::istream& is, TimedEvents& t )
{
    return internal::binSerializer().restore( is, t );
}

bool
TimedEvents::xml_archive( std::wostream& os, const TimedEvents& t )
{
    return internal::xmlSerializer("TimedEvents").archive( os, t );
}

bool
TimedEvents::xml_restore( std::wistream& is, TimedEvents& t )
{
    return internal::xmlSerializer("TimedEvents").restore( is, t );
}

const boost::uuids::uuid&
TimedEvents::clsid()
{
    static boost::uuids::uuid myclsid = boost::uuids::string_generator()( "{B70310BD-05B1-4C62-BFC3-9AF718FFB8CE}" );
    return myclsid;
}
