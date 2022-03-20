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

#include "tofchromatogrammethod.hpp"
#include "serializer.hpp"
#include <adportable/json/extract.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>

namespace adcontrols {

    class TofChromatogramMethod::impl {
    public:

        ~impl() {
        }

        impl() : formula_( "TIC" )
               , mass_( 0 )
               , massWindow_( 0 )
               , time_( 0 )
               , timeWindow_( 0 )
               , algo_( xic::ePeakAreaOnProfile )
               , protocol_( 0 )
               , id_( -1 )
               , enable_( false ) {
        }

        impl( const impl& t ) : formula_( t.formula_ )
                              , mass_( t.mass_ )
                              , massWindow_( t.massWindow_ )
                              , time_( t.time_ )
                              , timeWindow_( t.timeWindow_ )
                              , algo_( t.algo_ )
                              , protocol_( t.protocol_ )
                              , id_( t.id_ )
                              , enable_( t.enable_ ) {
        }

        std::string formula_;
        double mass_;
        double massWindow_;
        double time_;
        double timeWindow_;
        xic::eIntensityAlgorishm algo_;
        int32_t protocol_;  // 0, 1...
        int32_t id_;        // trace id := color index
        bool enable_;

    private:
        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int version ) {
            using namespace boost::serialization;

            ar & BOOST_SERIALIZATION_NVP( formula_ );
            ar & BOOST_SERIALIZATION_NVP( mass_ );
            ar & BOOST_SERIALIZATION_NVP( massWindow_ );
            ar & BOOST_SERIALIZATION_NVP( time_ );
            ar & BOOST_SERIALIZATION_NVP( timeWindow_ );
            ar & BOOST_SERIALIZATION_NVP( algo_ );
            if ( version >= 1 ) {
                ar & BOOST_SERIALIZATION_NVP( protocol_ );
                ar & BOOST_SERIALIZATION_NVP( id_ );
                ar & BOOST_SERIALIZATION_NVP( enable_ );
            }
        }
    };

    ////////// PORTABLE BINARY ARCHIVE //////////
    template<> void
    TofChromatogramMethod::serialize( portable_binary_oarchive& ar, const unsigned int version )
    {
        ar & *impl_;
    }

    template<> void
    TofChromatogramMethod::serialize( portable_binary_iarchive& ar, const unsigned int version )
    {
        ar & *impl_;
    }

    ///////// XML archive ////////
    template<> void
    TofChromatogramMethod::serialize( boost::archive::xml_woarchive& ar, const unsigned int )
    {
        ar & boost::serialization::make_nvp( "impl", *impl_ );
    }

    template<> void
    TofChromatogramMethod::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
    {
        ar & boost::serialization::make_nvp( "impl", *impl_ );
    }

}

BOOST_CLASS_VERSION( adcontrols::TofChromatogramMethod::impl, 1 )

using namespace adcontrols;

TofChromatogramMethod::TofChromatogramMethod() : impl_( new impl() )
{
}

TofChromatogramMethod::TofChromatogramMethod( const TofChromatogramMethod& t ) : impl_( new impl( *t.impl_ ) )
{
}

TofChromatogramMethod::~TofChromatogramMethod()
{
    delete impl_;
}

const std::string&
TofChromatogramMethod::formula() const
{
    return impl_->formula_;
}

void
TofChromatogramMethod::setFormula( const std::string& formula )
{
    impl_->formula_ = formula;
}

double
TofChromatogramMethod::mass() const
{
    return impl_->mass_;
}

void
TofChromatogramMethod::setMass( double value )
{
    impl_->mass_ = value;
}

double
TofChromatogramMethod::massWindow() const
{
    return impl_->massWindow_;
}

void
TofChromatogramMethod::setMassWindow( double value )
{
    impl_->massWindow_ = value;
}

double
TofChromatogramMethod::time() const
{
    return impl_->time_;
}

void
TofChromatogramMethod::setTime( double seconds )
{
    impl_->time_ = seconds;
}

const double
TofChromatogramMethod::timeWindow() const
{
    return impl_->timeWindow_;
}

void
TofChromatogramMethod::setTimeWindow( double seconds )
{
    impl_->timeWindow_ = seconds;
}

xic::eIntensityAlgorishm
TofChromatogramMethod::intensityAlgorithm() const
{
    return impl_->algo_;
}

void
TofChromatogramMethod::setIntensityAlgorithm( xic::eIntensityAlgorishm algo )
{
    impl_->algo_ = algo;
}

void
TofChromatogramMethod::setProtocol( int proto )
{
    impl_->protocol_ = proto;
}

int
TofChromatogramMethod::protocol() const
{
    return impl_->protocol_;
}

void
TofChromatogramMethod::setId( int id )
{
    impl_->id_ = id;
}

int
TofChromatogramMethod::id() const
{
    return impl_->id_;
}

bool
TofChromatogramMethod::enable() const
{
    return impl_->enable_;
}

void
TofChromatogramMethod::setEnable( bool enable )
{
    impl_->enable_ = enable;
}

namespace adcontrols {

    void
    tag_invoke( boost::json::value_from_tag, boost::json::value& jv, const TofChromatogramMethod& t )
    {
        jv = boost::json::object{ { "TofChromatogramMethod"
                , {
                    { "formula",      t.impl_->formula_ }
                    , { "mass",       t.impl_->mass_ }
                    , { "massWindow", t.impl_->massWindow_ }
                    , { "time",       t.impl_->time_ }
                    , { "algo",       int( t.impl_->algo_ ) }
                    , { "protocol",   t.impl_->protocol_ }
                    , { "enable",     t.impl_->enable_ }
                }
            }};
    }

    TofChromatogramMethod
    tag_invoke( boost::json::value_to_tag< TofChromatogramMethod >&, const boost::json::value& jv )
    {
        TofChromatogramMethod t;
        using namespace adportable::json;
        if ( jv.is_object() ) {
            auto obj = jv.as_object();
            extract( obj, t.impl_->formula_,     "formula" );
            extract( obj, t.impl_->mass_,        "mass" );
            extract( obj, t.impl_->massWindow_ , "massWindow" );
            extract( obj, t.impl_->time_ ,       "time" );
            int algo;
            extract( obj, algo,                  "algo" );
            t.impl_->algo_ = xic::eIntensityAlgorishm( algo );
            extract( obj, t.impl_->enable_,      "enable" );
        }
        return t;
    }
}
