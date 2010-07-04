//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "descriptions.h"

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/level.hpp>
#include <boost/serialization/version.hpp>

#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/base_object.hpp>

#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <sstream>
#include <vector>

using namespace adcontrols;

namespace boost {
    namespace serialization {

        template<class Archive>
        void serialize(Archive& ar, const Description& desc, const unsigned int version ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP(tv_sec_);
            ar & BOOST_SERIALIZATION_NVP(tv_usec_);
            ar & BOOST_SERIALIZATION_NVP(key_);
            ar & BOOST_SERIALIZATION_NVP(text_);
        }

    }
}

namespace adcontrols {
   namespace internal {

      class DescriptionsImpl {
	 public:
         ~DescriptionsImpl() {}
         DescriptionsImpl() {}
         DescriptionsImpl( const DescriptionsImpl& t ) : vec_(t.vec_) {}

	    typedef std::vector< Description > vector_type;

	    void add( const Description& desc, bool uniq );
	    inline size_t size() const { return vec_.size(); }
	    inline const Description& operator []( int idx ) { return vec_[idx]; }

	 private:
	    friend class boost::serialization::access;
	    template<class Archiver> void serialize(Archiver& ar, const unsigned int version) {
            if ( version > 0 )
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
   if ( pImpl_ != t.pImpl_ ) {
      delete pImpl_;
      pImpl_ = new DescriptionsImpl( *t.pImpl_ );
   }
}

void
Descriptions::add( const Description& desc, bool uniq )
{
   pImpl_->add( desc, uniq );
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

// template<class Archiver> void serialize(Archiver& ar, const unsigned int version); // {
template<> void
Descriptions::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
{
    if ( version >= 0 ) {
        ar << boost::serialization::make_nvp("Descriptions", pImpl_);
    }
}

template<> void
Descriptions::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
{
    if ( version >= 0 ) {
        ar >> boost::serialization::make_nvp("Descriptions", pImpl_);
    }
}


////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////

void
DescriptionsImpl::add( const Description& desc, bool uniq )
{
   if ( uniq ) {
      // to do
      // find desc.key from vec_, and remove it
   }
   vec_.push_back( desc );
}
