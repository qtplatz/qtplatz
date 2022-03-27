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

#include "tofchromatogramsmethod.hpp"
#include "tofchromatogrammethod.hpp"
#include "serializer.hpp"
#include <adportable/json_helper.hpp>
#include <adportable/json/extract.hpp>
#include <compiler/boost/workaround.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/json.hpp>
#include <string>
#include <map>
#include <vector>

namespace adcontrols {

    class TofChromatogramsMethod::impl {
    public:

        impl() : numberOfTriggers_( 100 )
               , refreshHistogram_( true )
               , vec_( 8 ) {
            initialize();
        }

        impl( const impl& t ) : numberOfTriggers_( t.numberOfTriggers_ )
                              , refreshHistogram_( t.refreshHistogram_ )
                              , vec_( t.vec_ ) {
            initialize();
        }

        void initialize() {
            const size_t sz = vec_.size();
            if ( sz < 8 )
                vec_.resize( 8 );
            for ( size_t i = 0; i < vec_.size(); ++i )
                vec_[ i ].setId( i );
        }

        size_t numberOfTriggers_;
        bool refreshHistogram_; // real-time monitoring parameter
        bool enableTIC_;
        xic::eIntensityAlgorishm algo_;
        std::vector< TofChromatogramMethod > vec_;

    private:
        friend class boost::serialization::access;
        template<class Archive>
            void serialize( Archive& ar, const unsigned int version ) {
            using namespace boost::serialization;

            ar & BOOST_SERIALIZATION_NVP( numberOfTriggers_ );
            if ( version >= 1 )
                ar & BOOST_SERIALIZATION_NVP( refreshHistogram_ );
            ar & BOOST_SERIALIZATION_NVP( vec_ );
            if ( version >= 2 ) {
                ar & BOOST_SERIALIZATION_NVP( enableTIC_ );
                ar & BOOST_SERIALIZATION_NVP( algo_ );
            }
        }
    };

}

BOOST_CLASS_VERSION( adcontrols::TofChromatogramsMethod::impl, 2 )

using namespace adcontrols;

TofChromatogramsMethod::~TofChromatogramsMethod()
{
    delete impl_;
}

TofChromatogramsMethod::TofChromatogramsMethod() : impl_( new impl() )
{
}

TofChromatogramsMethod::TofChromatogramsMethod( const TofChromatogramsMethod& t ) : impl_( new impl( *t.impl_ ) )
{
}

size_t
TofChromatogramsMethod::size() const
{
    return impl_->vec_.size();
}

void
TofChromatogramsMethod::clear()
{
    impl_->vec_.clear();
}

TofChromatogramsMethod&
TofChromatogramsMethod::operator << ( const TofChromatogramMethod& t )
{
    impl_->vec_.push_back( t );
    return *this;
}

TofChromatogramsMethod::iterator
TofChromatogramsMethod::begin()
{
    return impl_->vec_.begin();
}

TofChromatogramsMethod::iterator
TofChromatogramsMethod::end()
{
    return impl_->vec_.end();
}

TofChromatogramsMethod::const_iterator
TofChromatogramsMethod::begin() const
{
    return impl_->vec_.begin();
}

TofChromatogramsMethod::const_iterator
TofChromatogramsMethod::end() const
{
    return impl_->vec_.end();
}

size_t
TofChromatogramsMethod::numberOfTriggers() const
{
    return impl_->numberOfTriggers_;
}

void
TofChromatogramsMethod::setNumberOfTriggers( size_t value )
{
    impl_->numberOfTriggers_ = value;
}

bool
TofChromatogramsMethod::refreshHistogram() const
{
    return impl_->refreshHistogram_;
}

void
TofChromatogramsMethod::setRefreshHistogram( bool refresh )
{
    impl_->refreshHistogram_ = refresh;
}

std::tuple< bool, xic::eIntensityAlgorishm >
TofChromatogramsMethod::tic() const
{
    return { impl_->enableTIC_, impl_->algo_ };
}

void
TofChromatogramsMethod::setTIC( std::tuple< bool, xic::eIntensityAlgorishm >&& t )
{
    std::tie( impl_->enableTIC_, impl_->algo_ ) = std::move( t );
}

bool
TofChromatogramsMethod::archive( std::ostream& os, const TofChromatogramsMethod& t )
{
    return internal::binSerializer().archive( os, *t.impl_ );
}

//static
bool
TofChromatogramsMethod::restore( std::istream& is, TofChromatogramsMethod& t )
{
    return internal::binSerializer().restore( is, *t.impl_ );
}

//static
bool
TofChromatogramsMethod::xml_archive( std::wostream& os, const TofChromatogramsMethod& t )
{
    return internal::xmlSerializer("TofChromatogramsMethod").archive( os, *t.impl_ );
}

//static
bool
TofChromatogramsMethod::xml_restore( std::wistream& is, TofChromatogramsMethod& t )
{
    return internal::xmlSerializer("TofChromatogramsMethod").restore( is, *t.impl_ );
}

const boost::uuids::uuid&
TofChromatogramsMethod::clsid()
{
    static boost::uuids::uuid baseid = boost::uuids::string_generator()( "{3D2F180E-18E9-43D3-9A37-9E981B509CAA}" );
    static const boost::uuids::uuid myclsid = boost::uuids::name_generator( baseid )( "adcontrols::TofChromatogramsMethod" );
    return myclsid;
}

namespace adcontrols {

    void
    tag_invoke( boost::json::value_from_tag, boost::json::value& jv, const TofChromatogramsMethod& t )
    {
        jv = boost::json::object{{ "TofChromatogramsMethod"
                , {
                    { "numberOfTriggers",      t.impl_->numberOfTriggers_ }
                    , { "refreshHistogram",    t.impl_->refreshHistogram_ }
                    , { "enableTIC",           t.impl_->enableTIC_ }
                    , { "algoTIC",             int( t.impl_->algo_ ) }
                    , { "vec",                 t.impl_->vec_ }
                }
            }};
    }

    TofChromatogramsMethod
    tag_invoke( boost::json::value_to_tag< TofChromatogramsMethod >&, const boost::json::value& jv )
    {
        TofChromatogramsMethod t;
        using namespace adportable::json;
        auto tv = adportable::json_helper::find( jv, "TofChromatogramsMethod" );
        if ( tv.is_object() ) {
            try {
                auto obj = tv.as_object();
                extract( obj, t.impl_->numberOfTriggers_, "numberOfTriggers" );
                extract( obj, t.impl_->refreshHistogram_, "refreshHistogram" );
                extract( obj, t.impl_->enableTIC_,        "enableTIC" );
                int algo;
                extract( obj, algo,             "algoTIC" );
                t.impl_->algo_ = xic::eIntensityAlgorishm( algo );
                extract( obj, t.impl_->vec_,              "vec" );
            } catch ( std::exception& ex ) {
                BOOST_THROW_EXCEPTION(std::runtime_error("adportable/json/extract<TOFChromatogramsmethod> exception"));
            }
        }
        return t;
    }
}
