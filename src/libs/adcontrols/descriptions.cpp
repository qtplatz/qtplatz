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

#include <adportable_serializer/portable_binary_oarchive.hpp>
#include <adportable_serializer/portable_binary_iarchive.hpp>
#include <adportable/json/extract.hpp>
#include <adportable/json_helper.hpp>
#include <boost/json.hpp>

using namespace adcontrols;

namespace adcontrols {

    class descriptions::impl {
    public:
        ~impl() {}
        impl() {}
        impl( const impl& t ) : vec_(t.vec_) {}

        typedef std::vector< description > vector_type;

        void append( const description& desc, bool uniq );
        inline size_t size() const { return vec_.size(); }
        inline const description& operator []( size_t idx ) { return vec_[idx]; }
        inline operator const vector_type& () const { return vec_; }

    private:
        vector_type vec_;

        friend class adcontrols::descriptions;
        friend class boost::serialization::access;

        template<class Archiver> void serialize(Archiver& ar, const unsigned int version) {
            ar & BOOST_SERIALIZATION_NVP(vec_);
        }

        friend void tag_invoke( boost::json::value_from_tag, boost::json::value&, const descriptions& );
        friend descriptions tag_invoke( boost::json::value_to_tag< descriptions >&, const boost::json::value& );
    };
    //

    namespace {

        template< typename T >
        struct make_folder_name_t {
            std::basic_regex< T > regex_;
            bool negative_lookaround_;

            make_folder_name_t( const std::basic_string< T >& pattern
                                , bool negative_lookaround ) : regex_( pattern )
                                                             , negative_lookaround_( negative_lookaround ) {
            }

            std::basic_string< T > operator()( const std::vector< description >& vec ) const {
                std::basic_ostringstream< T > o;
                std::for_each( vec.rbegin(), vec.rend()
                               , [&] ( const description& d ){
                                   std::match_results< typename std::basic_string< T >::const_iterator > match;
                                   auto key = d.key<T>();
                                   bool result = std::regex_match( key, match, regex_ );
                                   if ( negative_lookaround_ )
                                       result = !result;
                                   if ( result ) {
                                       if ( !o.str().empty() )
                                           o << '/';
                                       o << d.text<T>();
                                   }
                               } );
                return o.str();
            }
        };
    }

}


BOOST_CLASS_VERSION(adcontrols::descriptions, 1)
BOOST_CLASS_VERSION(adcontrols::descriptions::impl, 1)

using namespace adcontrols::internal;

descriptions::~descriptions()
{
}

descriptions::descriptions() : impl_( std::make_unique< impl >() )
{
}

descriptions::descriptions( const descriptions& t )
{
	operator = ( t );
}

void
descriptions::operator = ( const descriptions& t )
{
    impl_->vec_ = t.impl_->vec_;
}

void
descriptions::append( const description& desc, bool uniq )
{
    impl_->append( desc, uniq );
}

size_t
descriptions::size() const
{
    return impl_->size();
}

const description&
descriptions::operator [] ( size_t idx ) const
{
    return (*impl_)[idx];
}

std::wstring
descriptions::toString() const
{
    std::wstring text;
    for ( auto& desc: *this )
        text += std::wstring( desc.text<wchar_t>() ) + L";";
    return text;
}

std::vector< description >::iterator
descriptions::begin()
{
    return impl_->vec_.begin();
}

std::vector< description >::iterator
descriptions::end()
{
    return impl_->vec_.end();
}

std::vector< description >::const_iterator
descriptions::begin() const
{
    return impl_->vec_.begin();
}

std::vector< description >::const_iterator
descriptions::end() const
{
    return impl_->vec_.end();
}

std::wstring
descriptions::make_folder_name( const std::wstring& regex, bool negative_lookaround ) const
{
    auto str = make_folder_name_t< wchar_t >( regex, negative_lookaround )( impl_->vec_ );
    // ADDEBUG() << "make_folder_name(" << regex << ")\n" << str << "\n" << this->toJson();
    return str;
}

std::string
descriptions::make_folder_name( const std::string& regex, bool negative_lookaround ) const
{
    auto str = make_folder_name_t< char >( regex, negative_lookaround )( impl_->vec_ );
    // ADDEBUG() << "make_folder_name(" << regex << ")\n" << str << "\n" << this->toJson();
    return str;
}

std::string
descriptions::toJson() const
{
    return boost::json::serialize( boost::json::value_from( *this ) );
}

std::optional< std::string >
descriptions::hasKey( const std::string& pattern ) const
{
    ADDEBUG() << "################ hasKey(" << pattern << ") ###############";
    for ( const auto& d: impl_->vec_ ) {
        std::match_results< std::string::const_iterator > match;
        auto key = d.key< char >();
        ADDEBUG() << "\tkey: " << key;
        if ( std::regex_search( key, match, std::regex( pattern ) ) ) {
            ADDEBUG() << "\t################ hasKey(" << pattern << ") found: " << d.keyValue();
            return d.text< char >();
        } else {
            ADDEBUG() << "\t################ not match #################";
        }
    }
    return {};
}

namespace adcontrols {
    /////////////////////// BINARY //////////////////////
    template<> void
    descriptions::serialize( portable_binary_oarchive& ar, const unsigned int version )
    {
        (void)version;
        ar & *impl_;
    }

    template<> void
    descriptions::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        (void)version;
        ar & *impl_;
    }

    /////////////////////// XML //////////////////////
    template<> void
    descriptions::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
    {
        (void)version;
        ar << boost::serialization::make_nvp("descriptions", *impl_);
    }

    template<> void
    descriptions::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        (void)version;
        ar >> boost::serialization::make_nvp("descriptions", *impl_);
    }


}; // namespace adcontrols
//-------------------------------------------------------------------------------------------


////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////

void
descriptions::impl::append( const description& desc, bool uniq )
{
   if ( uniq ) {
       auto it = std::find_if( vec_.begin(), vec_.end(), [&]( const auto& t ){ return t.keyValue().first == desc.keyValue().first; } );
       if ( it != vec_.end() ) {
           it->setValue( desc.keyValue().second );
           return;
       }
   }
   vec_.emplace_back( desc );
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

namespace adcontrols {

    void
    tag_invoke( boost::json::value_from_tag, boost::json::value& jv, const descriptions& t )
    {
        jv = { { "descriptions", t.impl_->vec_ } };
    }

    descriptions
    tag_invoke( boost::json::value_to_tag< descriptions >&, const boost::json::value& jv )
    {
        using namespace adportable::json;

        if ( jv.kind() == boost::json::kind::object ) {
            descriptions t;
            auto obj = jv.as_object();
            extract( obj, t.impl_->vec_, "descriptions" );
            return t;
        }
        return {};
    }

}
