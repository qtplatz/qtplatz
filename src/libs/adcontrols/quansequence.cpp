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
#if defined _MSC_VER
# pragma warning(disable:4996)
#endif

#include "quansequence.hpp"
#include "serializer.hpp"
#include "quansample.hpp"
#include "idaudit.hpp"
#include <adportable/debug.hpp>
#include <adportable/uuid.hpp>
#include <workaround/boost/uuid/uuid_io.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/base_object.hpp>
#include <workaround/boost/uuid/uuid_serialize.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <adportable/portable_binary_iarchive.hpp>

#if defined _MSC_VER
# pragma warning(default:4669)
#endif

namespace adcontrols {
    
    class QuanSequence::impl {
        
    public:
        ~impl() {
        }

        impl() {
        }

        impl( const impl& t ) : ident_( t.ident_ ), samples_( t.samples_ ) {
        }

        idAudit ident_;
        std::vector< QuanSample > samples_;
        std::wstring outfile_;
        std::wstring filename_;

        void operator << ( const QuanSample& t ) {
            int32_t rowid = int32_t( samples_.size() );
            samples_.push_back( t );
            samples_.back().sequence_uuid( ident_.uuid(), rowid );
        }

        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( ident_ );
            ar & BOOST_SERIALIZATION_NVP( samples_ );
            ar & BOOST_SERIALIZATION_NVP( outfile_ );
            ar & BOOST_SERIALIZATION_NVP( filename_ );
        }
    };
}

BOOST_CLASS_VERSION( adcontrols::QuanSequence::impl, 2 )

namespace adcontrols {

    ////////// PORTABLE BINARY ARCHIVE //////////
    template<> void
    QuanSequence::serialize( portable_binary_oarchive& ar, const unsigned int )
    {
        ar & *impl_;
    }

    template<> void
    QuanSequence::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        if ( version <= 1 )  {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( impl_->ident_ );
            ar & BOOST_SERIALIZATION_NVP( impl_->samples_ );
            ar & BOOST_SERIALIZATION_NVP( impl_->outfile_ );
            if ( version >= 1 )
                ar & BOOST_SERIALIZATION_NVP( impl_->filename_ );
        }
        else
            ar & *impl_;
    }

    ///////// XML archive ////////
    template<> ADCONTROLSSHARED_EXPORT void
    QuanSequence::serialize( boost::archive::xml_woarchive& ar, const unsigned int )
    {
        // if chagge tag name, XSLT also may need to be changed
        ar & boost::serialization::make_nvp("impl", *impl_);
    }

    template<> ADCONTROLSSHARED_EXPORT void
    QuanSequence::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        // if chagge tag name, XSLT also may need to be changed
        if ( version <= 1 )
            impl_->serialize( ar, version );
        else
            ar & boost::serialization::make_nvp("impl", *impl_);
    }
}


using namespace adcontrols;

QuanSequence::~QuanSequence()
{
}

QuanSequence::QuanSequence() : impl_( new impl() )
{
}

QuanSequence::QuanSequence( const QuanSequence& t ) : impl_( new impl( *t.impl_ ) )
{
}

QuanSequence&
QuanSequence::operator << ( const QuanSample& t )
{
    *impl_ << t;
    return *this;
}

QuanSequence::iterator
QuanSequence::begin()
{
    return impl_->samples_.begin();
}

QuanSequence::iterator
QuanSequence::end()
{
    return impl_->samples_.end();
}

QuanSequence::const_iterator
QuanSequence::begin() const
{
    return impl_->samples_.begin();
}

QuanSequence::const_iterator
QuanSequence::end() const
{
    return impl_->samples_.end();
}

size_t QuanSequence::size() const
{
    return impl_->samples_.size();
}

const boost::uuids::uuid&
QuanSequence::uuid() const
{
    return impl_->ident_.uuid();
}

const idAudit&
QuanSequence::ident() const
{
    return impl_->ident_;
}

const wchar_t *
QuanSequence::outfile() const
{
    return impl_->outfile_.c_str();
}

void QuanSequence::outfile( const wchar_t * file )
{
    impl_->outfile_ = file;
}

const wchar_t * QuanSequence::filename() const
{
    return impl_->filename_.c_str();
}

void
QuanSequence::filename( const wchar_t * file )
{
    impl_->filename_ = file;
}


//static
bool
QuanSequence::archive( std::ostream& os, const QuanSequence& t )
{
    return internal::binSerializer().archive( os, t );
}

//static
bool
QuanSequence::restore( std::istream& is, QuanSequence& t )
{
    return internal::binSerializer().restore( is, t );
}

//static
bool
QuanSequence::xml_archive( std::wostream& os, const QuanSequence& t )
{
    return internal::xmlSerializer("QuanSequence").archive( os, *t.impl_ );
}

//static
bool
QuanSequence::xml_restore( std::wistream& is, QuanSequence& t )
{
    return internal::xmlSerializer("QuanSequence").restore( is, *t.impl_ );
}
