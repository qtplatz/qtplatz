/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#include "descriptions.hpp"
#include <sstream>
#include <vector>
#include <regex>

#include <compiler/diagnostic_push.h>
#include <compiler/disable_unused_parameter.h>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/level.hpp>
#include <boost/serialization/version.hpp>

#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/base_object.hpp>

#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>

#include <adportable/portable_binary_oarchive.hpp>
#include <adportable/portable_binary_iarchive.hpp>

#include <compiler/diagnostic_pop.h>

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
			inline const Description& operator []( size_t idx ) { return vec_[idx]; }
            inline operator const vector_type& () const { return vec_; }

		private:
            friend class adcontrols::Descriptions;
			friend class boost::serialization::access;
			template<class Archiver> void serialize(Archiver& ar, const unsigned int version) {
			    (void)version;
			    ar & BOOST_SERIALIZATION_NVP(vec_);
			}
			vector_type vec_;
		};
		//

        template< typename char_t > struct make_folder_name {
            std::basic_regex< char_t > regex_;
            const DescriptionsImpl::vector_type& vec_;

            make_folder_name( const std::basic_string< char_t >& pattern
                              , const DescriptionsImpl::vector_type& vec ) : regex_( pattern ), vec_( vec ) {
            }

            std::basic_string< char_t > operator()() const {

                std::basic_string< char_t > name;

                // make it reverse order
                std::for_each( vec_.rbegin(), vec_.rend(), [&] ( const Description& d ){
                        
                        std::match_results< std::basic_string< wchar_t >::const_iterator > match;

                        if ( std::regex_match(d.key(), match, regex_) ) {
                            if ( !name.empty() )
                                name += char_t( ' ' );
                            name += d.text();
                        }

                    } );

                return name;
            }
        };
        
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
Descriptions::operator [] ( size_t idx ) const
{
   return (*pImpl_)[idx];
}

std::wstring
Descriptions::toString() const
{
    std::wstring text;
    for ( auto& desc: *this )
        text += desc.text() + L";";
    return text;
}

std::vector< Description >::iterator
Descriptions::begin()
{
    return pImpl_->vec_.begin();
}

std::vector< Description >::iterator
Descriptions::end()
{
    return pImpl_->vec_.end();
}

std::vector< Description >::const_iterator
Descriptions::begin() const
{
    return pImpl_->vec_.begin();
}

std::vector< Description >::const_iterator
Descriptions::end() const
{
    return pImpl_->vec_.end();    
}

std::wstring
Descriptions::make_folder_name( const std::wstring& regex ) const
{
    return internal::make_folder_name< wchar_t >( regex, *pImpl_ )();
}

namespace adcontrols {

    template<> void
    Descriptions::serialize( boost::archive::xml_oarchive& ar, const unsigned int version )
    {
        (void)version;
        ar << boost::serialization::make_nvp("Descriptions", pImpl_);
    }
    
    template<> void
    Descriptions::serialize( boost::archive::xml_iarchive& ar, const unsigned int version )
    {
        (void)version;
        ar >> boost::serialization::make_nvp("Descriptions", pImpl_);
    }

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
