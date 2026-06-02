/**************************************************************************
** Copyright (C) 2010-2026 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2026 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "jcb2009_peakresult.hpp"
#include <adcontrols/annotation.hpp>
#include <adcontrols/annotations.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msfinder.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/mspeakinfoitem.hpp>
#include <adcontrols/peak.hpp>
#include <adcontrols/peaks.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/segment_wrapper.hpp>
#include <adcontrols/targetingmethod.hpp>
#include <adportable/debug.hpp>
#include <adportable/json_helper.hpp>
#include <adportable/json/extract.hpp>
#include <adportfolio/folium.hpp>
#include <boost/json.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/format.hpp>
#include <algorithm>
#include <iterator>

namespace adcontrols {

    namespace jcb2009 {

        Peak::Peak() : pkid_( 0 )
                     , peakTime_  ( 0 )
                     , peakStart_ ( 0 )
                     , peakEnd_   ( 0 )
        {
        }
        Peak::Peak( const Peak& t ) : pkid_( t.pkid_ )
                                    , peakTime_( t.peakTime_ )
                                    , peakStart_( t.peakStart_ )
                                    , peakEnd_( t.peakEnd_ )
        {
        }

        Peak::Peak( int pkid
                    , double peakTime
                    , double peakStart
                    , double peakEnd ) : pkid_( pkid )
                                       , peakTime_( peakTime )
                                       , peakStart_( peakStart )
                                       , peakEnd_( peakEnd )
        {
        }

        class dataSource::impl {
        public:
            impl() : mass_ ( 0 )
                     , mass_width_ ( 0 )
                     , protocol_   ( -1 ) {
            }
            impl( const impl& t ) : folium_( t.folium_ )
                                  , dataGuid_( t.dataGuid_ )
                                  , mass_( t.mass_ )
                                  , mass_width_( t.mass_width_ )
                                  , protocol_( t.protocol_ )
                                  , pk_( t.pk_ ) {
            }

            impl( const std::pair< std::string, boost::uuids::uuid >& foliumGuid
                  , const boost::uuids::uuid& dataGuid
                  , const adcontrols::Peak& pk
                  , const std::string& formula
                  , const std::string& adduct
                  , double mass
                  , double mass_width
                  , int32_t protocol ) : folium_( std::move( foliumGuid ) )
                                       , dataGuid_( dataGuid )
                                       , mass_( mass )
                                       , mass_width_ ( mass_width )
                                       , protocol_( protocol )
                                       , formula_( formula )
                                       , adduct_ ( adduct ) {
                pk_ = Peak{ pk.peakId(), pk.peakTime(), pk.startTime(), pk.endTime() };
            }

            std::pair< std::string, boost::uuids::uuid > folium_;
            boost::uuids::uuid dataGuid_;
            double mass_;
            double mass_width_;
            int32_t protocol_;
            std::string formula_;
            std::string adduct_;
            Peak pk_;
        };

        dataSource::~dataSource()
        {
        }

        dataSource::dataSource() : impl_( std::make_unique< impl >() )
        {
        }

        dataSource::dataSource( const dataSource& t ) : impl_( std::make_unique< impl >( *t.impl_ ) )
        {
        }

        const dataSource&
        dataSource::operator = ( const dataSource& t )
        {
            (*this) = t;
            return *this;
        }

        dataSource::dataSource( std::pair< std::string, boost::uuids::uuid >&& foliumGuid
                                , const boost::uuids::uuid& dataGuid
                                , const adcontrols::Peak& pk
                                , const std::optional< std::string >& formula
                                , const std::string& adduct
                                , double mass
                                , double mass_width
                                , int32_t protocol ) : impl_( std::make_unique< impl >( foliumGuid
                                                                                        , dataGuid
                                                                                        , pk
                                                                                        , formula ? *formula : ""
                                                                                        , adduct
                                                                                        , mass
                                                                                        , mass_width
                                                                                        , protocol ) )
        {
        }

        const std::pair< std::string, boost::uuids::uuid >&
        dataSource::folium() const
        {
            return impl_->folium_;
        }

        const boost::uuids::uuid&
        dataSource::dataGuid() const
        {
            return impl_->dataGuid_;
        }

        double
        dataSource::mass() const
        {
            return impl_->mass_;
        }

        double
        dataSource::mass_width() const
        {
            return impl_->mass_width_;
        }

        int32_t
        dataSource::protocol() const
        {
            return impl_->protocol_;
        }

        std::string
        dataSource::formula() const
        {
            return impl_->formula_;
        }

        std::string
        dataSource::adduct() const
        {
            return impl_->adduct_;
        }

        const Peak&
        dataSource::pk() const
        {
            return impl_->pk_;
        }

        void
        tag_invoke( const boost::json::value_from_tag, boost::json::value& jv, const Peak& t )
        {
            jv = boost::json::object{
                { "Peak", {  { "pkid", t.pkid_ }
                             , { "peakTime", t.peakTime_ }
                             , { "peakStart", t.peakStart_ }
                             , { "peakEnd", t.peakEnd_ } } }};
        }
        Peak
        tag_invoke( const boost::json::value_to_tag< Peak >&, const boost::json::value& jv )
        {
            Peak t;
            if ( jv.kind() == boost::json::kind::object ) {
                if ( auto peak = jv.as_object().if_contains( "Peak" ) ) {
                    using namespace adportable::json;

                    auto obj = peak->as_object();
                    extract( obj, t.pkid_, "pkid" );
                    extract( obj, t.peakTime_, "peakTime" );
                    extract( obj, t.peakStart_, "peakStart" );
                    extract( obj, t.peakEnd_, "peakEnd" );
                }
            }
            return t;
        }

        ///
        void
        tag_invoke( const boost::json::value_from_tag, boost::json::value& jv, const dataSource& t )
        {
            jv = boost::json::object{
                { "dataSource", {
                        { "folium", {
                                { "name", t.impl_->folium_.first }
                                ,{ "uuid", boost::uuids::to_string( t.impl_->folium_.second ) }
                            } } // "folium"
                        , { "dataGuid", boost::uuids::to_string( t.impl_->dataGuid_ ) }
                        , { "mass", t.impl_->mass_ }
                        , { "mass_width", t.impl_->mass_ }
                        , { "protocol", t.impl_->protocol_ }
                        , { "Peak", boost::json::value_from( t.impl_->pk_ ) }
                    }
                } // dataSource
            };
        }

        dataSource
        tag_invoke( const boost::json::value_to_tag< dataSource >&, const boost::json::value& jv )
        {
            dataSource t;
            if ( jv.kind() == boost::json::kind::object ) {
                if ( auto p = jv.as_object().if_contains( "dataSource" ) ) {
                    using namespace adportable::json;
                    auto obj = p->as_object();
                    if ( auto pfolium = obj.if_contains( "folium" ) ) {
                        auto fobj = pfolium->as_object();
                        extract( fobj, t.impl_->folium_.first, "name" );
                        extract( fobj, t.impl_->folium_.second, "uuid" );
                    }
                    extract( obj, t.impl_->dataGuid_, "dataGuid" );
                    extract( obj, t.impl_->mass_, "mass" );
                    extract( obj, t.impl_->mass_width_, "mass_width" );
                    extract( obj, t.impl_->protocol_, "protocol" );
                    extract( obj, t.impl_->pk_, "Peak" );
                }
            }
            // ADDEBUG() << "####### value_to<dataSource> loopback #########\n\n" << boost::json::value_from( t ) << "\n\n";
            return t;
        }
    } // namesapce jcb2009


    jcb2009_peakresult::jcb2009_peakresult() : chro_tR_(0)
                                             , chro_peak_width_(0)
                                             , chro_peak_area_(0)
                                             , chro_peak_height_(0)
                                             , chro_generator_mass_(0)
                                             , chro_generator_mass_width_(0)
                                             , matched_mass_(0)
                                             , matched_mass_width_(0)
                                             , matched_mass_height_(0)
                                             , protocol_(0)
                                             , dataSource_{ "", {0} }
                                             , dataGuid_({})
                                             , pkid_( -1 )
    {
    }

    jcb2009_peakresult::jcb2009_peakresult( const jcb2009_peakresult& t )
        : chro_tR_( t.chro_tR_ )
        , chro_peak_width_( t.chro_peak_width_ )
        , chro_peak_area_( t.chro_peak_area_ )
        , chro_peak_height_( t.chro_peak_height_ )
        , chro_peak_name_( t.chro_peak_name_ )
        , chro_generator_mass_( t.chro_generator_mass_ )
        , chro_generator_mass_width_( t.chro_generator_mass_width_ )
        , matched_mass_( t.matched_mass_ )
        , matched_mass_width_( t.matched_mass_width_ )
        , matched_mass_height_( t.matched_mass_height_ )
        , protocol_( t.protocol_ )
        , dataSource_( t.dataSource_ )
        , dataGuid_( t.dataGuid_ )
        , pkid_ ( t.pkid_ )
    {
    }

    jcb2009_peakresult::jcb2009_peakresult( double mass
                                            , double mass_width
                                            , int protocol
                                            , const adcontrols::Peak& peak
                                            , std::pair< std::string, boost::uuids::uuid >&& folder
                                            , const boost::uuids::uuid& dataGuid )
        : chro_tR_         ( peak.peakTime() )
        , chro_peak_width_ ( peak.peakWidth() )
        , chro_peak_area_  ( peak.peakArea() )
        , chro_peak_height_( peak.peakHeight() )
        , chro_peak_name_  ( peak.name() )
        , chro_generator_mass_( mass )
        , chro_generator_mass_width_( mass_width )
        , matched_mass_( 0 )
        , matched_mass_width_( 0 )
        , matched_mass_height_( 0 )
        , protocol_( protocol )
        , dataSource_      ( folder )
        , dataGuid_ ( dataGuid )
        , pkid_ ( peak.peakId() )
    {
    }

    void
    jcb2009_peakresult::set_found_mass( const adcontrols::MSPeakInfoItem& item )
    {
        matched_mass_        = item.mass();
        matched_mass_width_  = item.widthHH();
        matched_mass_height_ = item.height();
    }

    int jcb2009_peakresult::protocol() const
    {
        return protocol_;
    }

    double jcb2009_peakresult::tR() const                    { return chro_tR_; }
    double jcb2009_peakresult::peak_width() const            { return chro_peak_width_; }
    double jcb2009_peakresult::peak_height() const           { return chro_peak_height_; }
    double jcb2009_peakresult::peak_area() const             { return chro_peak_area_; }
    const std::string& jcb2009_peakresult::peak_name() const { return chro_peak_name_; }
    double jcb2009_peakresult::generator_mass() const        { return chro_generator_mass_; }
    double jcb2009_peakresult::generator_mass_width() const  { return chro_generator_mass_width_; }
    double jcb2009_peakresult::matched_mass() const          { return matched_mass_; }
    double jcb2009_peakresult::matched_mass_width() const    { return matched_mass_width_; }
    double jcb2009_peakresult::matched_mass_height() const   { return matched_mass_height_; }

    bool jcb2009_peakresult::operator < ( const jcb2009_peakresult& t ) const { return matched_mass_ < t.matched_mass_; }

    void
    tag_invoke( const boost::json::value_from_tag, boost::json::value& jv, const jcb2009_peakresult& t )
    {
        jv = { { "jcb2009_peakresult"
                     , {{   "tR",                   t.chro_tR_ }
                        , { "peak_width",           t.chro_peak_width_ }
                        , { "peak_area",            t.chro_peak_area_ }
                        , { "peak_height",          t.chro_peak_height_ }
                        , { "peak_name",            t.chro_peak_name_ }
                        , { "generator_mass",       t.chro_generator_mass_ }
                        , { "generator_mass_width", t.chro_generator_mass_width_ }
                        , { "matched_mass",         t.matched_mass_ }
                        , { "matched_mass_width",   t.matched_mass_width_ }
                        , { "protocol",             t.protocol_ }
                        , { "dataSource_name",      t.dataSource_.first }
                        , { "dataSource_uuid",      boost::uuids::to_string(t.dataSource_.second) }
                        , { "dataGuid",      boost::uuids::to_string( t.dataGuid_ ) }
                }
            }
        };
    }

    jcb2009_peakresult
    tag_invoke( const boost::json::value_to_tag< jcb2009_peakresult >&, const boost::json::value& jv )
    {
        jcb2009_peakresult t;
        using namespace adportable::json;

        if ( jv.kind() == boost::json::kind::object ) {
            if ( auto peakresult = jv.as_object().if_contains( "jcb2009_peakresult" ) ) {
                auto obj = peakresult->as_object();

                extract( obj, t.chro_tR_,                     "tR" );
                extract( obj, t.chro_peak_width_,             "peak_width");
                extract( obj, t.chro_peak_area_,              "peak_area");
                extract( obj, t.chro_peak_height_,            "peak_height");
                extract( obj, t.chro_peak_name_,              "peak_name");
                extract( obj, t.chro_generator_mass_,         "generator_mass");
                extract( obj, t.chro_generator_mass_width_,   "generator_mass_width");
                extract( obj, t.matched_mass_,                "matched_mass");
                extract( obj, t.matched_mass_width_,          "matched_mass_width");
                extract( obj, t.protocol_,                    "protocol");
                extract( obj, t.dataSource_.first,            "dataSource_name");
                extract( obj, t.dataSource_.second,           "dataSource_uuid");
                extract( obj, t.dataGuid_,                    "dataGuid");
            }
        }
        return t;
    }
    //////////////////////////////////////////
}
