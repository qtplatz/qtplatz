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
#include "serializer.hpp"
#include <sstream>
#include <vector>
#include <regex>

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

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <adportable/portable_binary_oarchive.hpp>
#include <adportable/portable_binary_iarchive.hpp>

using namespace adcontrols;

namespace adcontrols {
	namespace internal {

		class descriptionsImpl {
		public:
			~descriptionsImpl() {}
			descriptionsImpl() {}
			descriptionsImpl( const descriptionsImpl& t ) : vec_(t.vec_) {}

			typedef std::vector< description > vector_type;

			void append( const description& desc, bool uniq );
			inline size_t size() const { return vec_.size(); }
			inline const description& operator []( size_t idx ) { return vec_[idx]; }
            inline operator const vector_type& () const { return vec_; }

		private:
            friend class adcontrols::descriptions;
			friend class boost::serialization::access;
			template<class Archiver> void serialize(Archiver& ar, const unsigned int version) {
			    (void)version;
			    ar & BOOST_SERIALIZATION_NVP(vec_);
			}
			vector_type vec_;
		};
		//

        struct make_folder_name {
            std::basic_regex< wchar_t > regex_;
            const descriptionsImpl::vector_type& vec_;

            make_folder_name( const std::wstring& pattern
                              , const descriptionsImpl::vector_type& vec ) : regex_( pattern ), vec_( vec ) {
            }

            std::wstring operator()() const {

                std::wstring name;

                std::for_each( vec_.rbegin(), vec_.rend(), [&] ( const description& d ){

                        std::match_results< std::wstring::const_iterator > match;
                        std::wstring key = d.key();
                        if ( std::regex_match( key, match, regex_ ) ) {
                            if ( !name.empty() )
                                name += L' ';
                            name += d.text();
                        }

                    } );

                return name;
            }
        };

	}
}

BOOST_CLASS_VERSION(adcontrols::descriptions, 1)
BOOST_CLASS_VERSION(adcontrols::internal::descriptionsImpl, 1)

using namespace adcontrols::internal;

descriptions::~descriptions()
{
    delete pImpl_;
}

descriptions::descriptions() : pImpl_(0)
{
    pImpl_ = new descriptionsImpl();
}

descriptions::descriptions( const descriptions& t )
{
	operator = ( t );
}

void
descriptions::operator = ( const descriptions& t )
{
    if ( pImpl_ != t.pImpl_ ) {
        delete pImpl_;
        pImpl_ = new descriptionsImpl( *t.pImpl_ );
    }
}

void
descriptions::append( const description& desc, bool uniq )
{
    pImpl_->append( desc, uniq );
}

descriptions&
descriptions::operator << ( const description& desc )
{
    pImpl_->append( desc, false );
    return *this;
}

size_t
descriptions::size() const
{
    return pImpl_->size();
}

const description&
descriptions::operator [] ( size_t idx ) const
{
   return (*pImpl_)[idx];
}

std::wstring
descriptions::toString() const
{
    std::wstring text;
    for ( auto& desc: *this )
        text += std::wstring( desc.text() ) + L";";
    return text;
}

std::vector< description >::iterator
descriptions::begin()
{
    return pImpl_->vec_.begin();
}

std::vector< description >::iterator
descriptions::end()
{
    return pImpl_->vec_.end();
}

std::vector< description >::const_iterator
descriptions::begin() const
{
    return pImpl_->vec_.begin();
}

std::vector< description >::const_iterator
descriptions::end() const
{
    return pImpl_->vec_.end();
}

std::wstring
descriptions::make_folder_name( const std::wstring& regex ) const
{
    return internal::make_folder_name( regex, *pImpl_ )();
}

std::string
descriptions::toJson() const
{
    boost::property_tree::ptree pt;
    for ( const auto& desc: *this ) {
        std::string key, value;
        std::tie(key, value) = desc.keyValue();
        pt.put( key, value );
    }
    std::ostringstream o;
    boost::property_tree::write_json( o, pt );
    return o.str();
}

namespace adcontrols {
    /////////////////////// BINARY //////////////////////
    template<> void
    descriptions::serialize( portable_binary_oarchive& ar, const unsigned int version )
    {
        (void)version;
        ar & *pImpl_;
    }

    template<> void
    descriptions::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        (void)version;
        ar & *pImpl_;
    }

    /////////////////////// XML //////////////////////
    template<> void
    descriptions::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
    {
        (void)version;
        ar << boost::serialization::make_nvp("descriptions", pImpl_);
    }

    template<> void
    descriptions::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        (void)version;
        ar >> boost::serialization::make_nvp("descriptions", pImpl_);
    }


}; // namespace adcontrols
//-------------------------------------------------------------------------------------------


////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////

void
descriptionsImpl::append( const description& desc, bool uniq )
{
   if ( uniq ) {
      // to do
      // find desc.key from vec_, and remove it
   }
   vec_.push_back( desc );
}

//static
bool
descriptions::xml_archive( std::wostream& os, const descriptions& t )
{
    return internal::xmlSerializer("descriptions").archive( os, t );
}

//static
bool
descriptions::xml_restore( std::wistream& is, descriptions& t )
{
    return internal::xmlSerializer("descriptions").restore( is, t );
}
