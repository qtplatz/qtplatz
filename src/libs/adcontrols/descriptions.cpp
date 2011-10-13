/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#include "descriptions.hpp"
#include <sstream>
#include <vector>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/level.hpp>
#include <boost/serialization/version.hpp>

#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/base_object.hpp>

#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>

#if defined _MSC_VER
# pragma warning( disable : 4996 )
#endif
//# include <boost/archive/binary_oarchive.hpp>
//# include <boost/archive/binary_iarchive.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <adportable/portable_binary_iarchive.hpp>

using namespace adcontrols;

namespace adcontrols {
	namespace internal {

		class DescriptionsImpl {
		public:
			~DescriptionsImpl() {}
			DescriptionsImpl() {}
			DescriptionsImpl( const DescriptionsImpl& t ) : vec_(t.vec_) {}

			typedef std::vector< Description > vector_type;

			void append( const Description& desc, bool uniq );
			inline size_t size() const { return vec_.size(); }
			inline const Description& operator []( int idx ) { return vec_[idx]; }

		private:
			friend class boost::serialization::access;
			template<class Archiver> void serialize(Archiver& ar, const unsigned int version) {
			    (void)version;
			    ar & BOOST_SERIALIZATION_NVP(vec_);
			}
			vector_type vec_;
		};
		//
	}
}

BOOST_CLASS_VERSION(adcontrols::Descriptions, 1)
BOOST_CLASS_VERSION(adcontrols::internal::DescriptionsImpl, 1)

using namespace adcontrols::internal;

Descriptions::~Descriptions()
{
    delete pImpl_;
}

Descriptions::Descriptions() : pImpl_(0)
{
    pImpl_ = new DescriptionsImpl();
}

Descriptions::Descriptions( const Descriptions& t )
{
	operator = ( t );
}

void
Descriptions::operator = ( const Descriptions& t )
{
   if ( pImpl_ != t.pImpl_ ) {
      delete pImpl_;
      pImpl_ = new DescriptionsImpl( *t.pImpl_ );
   }
}

void
Descriptions::append( const Description& desc, bool uniq )
{
   pImpl_->append( desc, uniq );
}

size_t
Descriptions::size() const
{
   return pImpl_->size();
}

const Description& 
Descriptions::operator [] ( int idx ) const
{
   return (*pImpl_)[idx];
}

std::wstring
Descriptions::saveXml() const
{
    std::wostringstream o;
    boost::archive::xml_woarchive ar( o );
    ar << boost::serialization::make_nvp("Descriptions", pImpl_);
    return o.str();
}

void
Descriptions::loadXml( const std::wstring& xml )
{
    std::wistringstream in( xml );
    boost::archive::xml_wiarchive ar( in );
    ar >> boost::serialization::make_nvp("Descriptions", pImpl_);
}

namespace adcontrols {

    template<> void
    Descriptions::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
    {
	(void)version;
	ar << boost::serialization::make_nvp("Descriptions", pImpl_);
    }
    
    template<> void
    Descriptions::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
	(void)version;
	ar >> boost::serialization::make_nvp("Descriptions", pImpl_);
    }
    
    template<> void
    Descriptions::serialize( portable_binary_oarchive& ar, const unsigned int version )
    {
	(void)version;
	ar & *pImpl_;
    }
    
    template<> void
    Descriptions::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
	(void)version;
	ar & *pImpl_;
    }
}; // namespace adcontrols
//-------------------------------------------------------------------------------------------


////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////

void
DescriptionsImpl::append( const Description& desc, bool uniq )
{
   if ( uniq ) {
      // to do
      // find desc.key from vec_, and remove it
   }
   vec_.push_back( desc );
}
