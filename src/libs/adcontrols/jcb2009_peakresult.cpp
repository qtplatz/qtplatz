/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC, Toin, Mie Japan
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
// #include "generator_property.hpp"
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
#include <boost/format.hpp>
#include <algorithm>
#include <iterator>

namespace adcontrols {

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
    {
    }

    jcb2009_peakresult::jcb2009_peakresult( std::tuple< double, double, int >&& gen
                                            , const adcontrols::Peak& peak
                                            , std::pair< std::string, boost::uuids::uuid >&& folder )
    {
        chro_generator_mass_        = std::get< 0 >( gen ); // mass
        chro_generator_mass_width_  = std::get< 1 >( gen ); // mass
        protocol_                   = std::get< 2 >( gen );

        chro_tR_                    = peak.peakTime();
        chro_peak_width_            = peak.peakWidth();
        chro_peak_area_             = peak.peakArea();
        chro_peak_height_           = peak.peakHeight();
        chro_peak_name_             = peak.name();
        dataSource_                 = folder;
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
            }
        }
        return t;
    }
    //////////////////////////////////////////
}
