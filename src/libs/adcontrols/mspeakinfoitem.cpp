// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include "mspeakinfoitem.hpp"
#include "serializer.hpp"
#include <adportable/utf.hpp>
#include <adportable/waveform_peakfinder.hpp>
#include <adportable/json/extract.hpp>
#include <cstring>
#if BOOST_VERSION >= 107500
# include <boost/json.hpp>
#endif

using namespace adcontrols;
using adportable::utf;

MSPeakInfoItem::~MSPeakInfoItem(void)
{
}

MSPeakInfoItem::MSPeakInfoItem(void)
    : peak_index_(0)
    , peak_start_index_(0)
    , peak_end_index_(0)
    , base_height_(0)
    , mass_(0)
    , area_(0)
    , height_(0)
    , time_from_mass_(0)
    , time_from_time_(0)
    , HH_left_mass_(0)
    , HH_right_mass_(0)
    , HH_left_time_(0)
    , HH_right_time_(0)
    , centroid_left_mass_(0)
    , centroid_right_mass_(0)
    , centroid_left_time_(0)
    , centroid_right_time_(0)
    , centroid_threshold_(0)
    , is_visible_( true )
    , is_reference_( false )
    , mode_( boost::none )
{
}

MSPeakInfoItem::MSPeakInfoItem( const MSPeakInfoItem& t )
    : peak_index_( t.peak_index_)
    , peak_start_index_( t.peak_start_index_)
    , peak_end_index_( t.peak_end_index_)
    , base_height_( t.base_height_)
    , mass_( t.mass_)
    , area_( t.area_)
    , height_( t.height_)
    , time_from_mass_( t.time_from_mass_)
    , time_from_time_( t.time_from_time_)
    , HH_left_mass_( t.HH_left_mass_)
    , HH_right_mass_( t.HH_right_mass_)
    , HH_left_time_( t.HH_left_time_)
    , HH_right_time_( t.HH_right_time_)
    , centroid_left_mass_( t.centroid_left_mass_)
    , centroid_right_mass_( t.centroid_right_mass_)
    , centroid_left_time_( t.centroid_left_time_)
    , centroid_right_time_( t.centroid_right_time_)
    , centroid_threshold_( t.centroid_threshold_)
    , is_visible_( t.is_visible_)
    , is_reference_( t.is_reference_)
    , formula_( t.formula_ )
    , annotation_( t.annotation_ )
    , mode_( t.mode_ )
{
}

MSPeakInfoItem::MSPeakInfoItem( const adportable::waveform_peakfinder::peakinfo& pk
                                , size_t idx
                                , double dbase
                                , bool isTime
                                , std::function<double(double)> mass_assignee )
    : peak_index_( idx )
    , peak_start_index_( pk.spos )
    , peak_end_index_( pk.epos )
    , base_height_( dbase )
    , mass_(0)
    , area_(0)
    , height_( pk.height )
    , time_from_mass_(0)
    , time_from_time_(0)
    , HH_left_mass_(0)
    , HH_right_mass_(0)
    , HH_left_time_(0)
    , HH_right_time_(0)
    , centroid_left_mass_(0)
    , centroid_right_mass_(0)
    , centroid_left_time_(0)
    , centroid_right_time_(0)
    , centroid_threshold_(0)
    , is_visible_( true )
    , is_reference_( false )
    , mode_( boost::none )
{
    set_centroid_threshold( pk.height / 2 + dbase );

    if  ( isTime ) {
        set_time( pk.centreX, pk.xleft, pk.xright, true );  // time compute from time
        set_time( pk.centreX, pk.xleft, pk.xright, false ); // workaround, copy same value

        if ( mass_assignee )
            set_mass( mass_assignee( pk.centreX ), mass_assignee( pk.xleft ), mass_assignee( pk.xright ) );

        set_width_hh_lr( centroid_left(), centroid_right(), false ); // m/z
        set_width_hh_lr( pk.xleft, pk.xright, true ); // time
        set_centroid_left( pk.xleft, true );
        set_centroid_right( pk.xright, true );
    }
}

unsigned int
MSPeakInfoItem::peak_index() const
{
    return peak_index_;
}

void
MSPeakInfoItem::set_peak_index( unsigned int idx )
{
    peak_index_ = idx;
}

unsigned int
MSPeakInfoItem::peak_start_index() const
{
    return peak_start_index_;
}

void
MSPeakInfoItem::set_peak_start_index( unsigned int idx )
{
    peak_start_index_ = idx;
}

unsigned int
MSPeakInfoItem::peak_end_index() const
{
    return peak_end_index_;
}

void
MSPeakInfoItem::set_peak_end_index( unsigned int idx )
{
    peak_end_index_ = idx;
}

double
MSPeakInfoItem::base_height() const
{
    return base_height_;
}

void
MSPeakInfoItem::set_base_height( double h )
{
    base_height_ = h;
}

double
MSPeakInfoItem::mass() const
{
    return mass_;
}

double
MSPeakInfoItem::area() const
{
    return area_;
}

void
MSPeakInfoItem::set_area( double a )
{
    area_ = a;
}

double
MSPeakInfoItem::height() const
{
    return height_;
}

void
MSPeakInfoItem::set_height( double h )
{
    height_ = h;
}

double
MSPeakInfoItem::widthHH( bool time ) const
{
    return time ? ( HH_right_time_ - HH_left_time_ ) : ( HH_right_mass_ - HH_left_mass_ );
}

double
MSPeakInfoItem::time( bool time ) const
{
    return time ? time_from_time_ : time_from_mass_;
}

double
MSPeakInfoItem::centroid_left( bool time ) const
{
    return time ? centroid_left_time_ : centroid_left_mass_;
}

double
MSPeakInfoItem::centroid_right( bool time ) const
{
    return time ? centroid_right_time_ : centroid_right_mass_;
}

void
MSPeakInfoItem::set_centroid_left( double t, bool time )
{
    if ( time )
        centroid_left_time_ = t;
    else
        centroid_left_mass_ = t;
}

void
MSPeakInfoItem::set_centroid_right( double t, bool time )
{
    if ( time )
        centroid_right_time_ = t;
    else
        centroid_right_mass_ = t;
}

double
MSPeakInfoItem::centroid_threshold() const
{
    return centroid_threshold_;
}

void
MSPeakInfoItem::set_centroid_threshold( double t )
{
    centroid_threshold_ = t;
}

double
MSPeakInfoItem::hh_left_time() const
{
    return HH_left_time_;
}

double
MSPeakInfoItem::hh_right_time() const
{
    return HH_right_time_;
}

const std::string&
MSPeakInfoItem::formula() const
{
    return formula_;
}

void
MSPeakInfoItem::formula( const std::string& formula )
{
    formula_ = formula;
}

const std::string&
MSPeakInfoItem::annotation() const
{
    return annotation_;
}

void
MSPeakInfoItem::annotation( const std::wstring& v )
{
    annotation_ = utf::to_utf8( v );
}

void
MSPeakInfoItem::annotation( const std::string& v )
{
    annotation_ = v;
}

bool
MSPeakInfoItem::visible() const
{
    return is_visible_;
}

void
MSPeakInfoItem::visible( bool f )
{
    is_visible_ = f;
}


void
MSPeakInfoItem::set_mass( double mass, double left, double right )
{
    mass_ = mass;
    centroid_left_mass_ = left;
    centroid_right_mass_ = right;
}

void
MSPeakInfoItem::set_time( double time, double left, double right, bool from_time )
{
    if ( from_time )
        time_from_time_ = time;
    else
        time_from_mass_ = time;

    centroid_left_time_ = left;
    centroid_right_time_ = right;
}

void
MSPeakInfoItem::set_width_hh_lr( double left, double right, bool time )
{
    if ( time ) {
        HH_left_time_ = left;
        HH_right_time_ = right;
    } else {
        HH_left_mass_ = left;
        HH_right_mass_ = right;
    }
}

void
MSPeakInfoItem::assign_mass( double mass )
{
    // this will cause when re-assign calibration to centroid spectrum occured
    double d = mass - mass_;

    mass_ = mass;
    centroid_left_mass_ += d;
    centroid_right_mass_ += d;
    HH_left_mass_ += d;
    HH_right_mass_ += d;
}

boost::optional< int >
MSPeakInfoItem::mode() const
{
    return mode_;
    // return mode_ ? std::optional<int>(*mode_) : std::nullopt;
}

bool
MSPeakInfoItem::is_reference() const
{
    return is_reference_;
}

void
MSPeakInfoItem::is_reference( bool f )
{
    is_reference_ = f;
}

void
MSPeakInfoItem::set_mode( boost::optional< int >&& mode )
{
    mode_ = mode;
    //mode_ = mode ? boost::optional<int32_t>(*mode) : boost::none;
}

//static
bool
MSPeakInfoItem::xml_archive( std::wostream& os, const MSPeakInfoItem& t )
{
    return internal::xmlSerializer("MSPeakInfoItem").archive( os, t );
}

//static
bool
MSPeakInfoItem::xml_restore( std::wistream& is, MSPeakInfoItem& t )
{
    return internal::xmlSerializer("MSPeakInfoItem").restore( is, t );
}

std::string
MSPeakInfoItem::toJson() const
{
    auto jv = boost::json::value_from( *this );
    return boost::json::serialize( jv );
}

// static
boost::optional< MSPeakInfoItem >
MSPeakInfoItem::fromJson( const std::string& json )
{
#if BOOST_VERSION >= 107500
    boost::system::error_code ec;
    auto jv = boost::json::parse( json, ec );
    if ( ! ec )
        return fromJson( jv );
#endif
    return {};
}

boost::optional< MSPeakInfoItem >
MSPeakInfoItem::fromJson( const boost::json::value& jv )
{
    return boost::json::value_to< MSPeakInfoItem >( jv );
}

MSPeakInfoItem
MSPeakInfoItem::tag_invoke( boost::json::value const& jv )
{
    ADDEBUG() << "===================== tag_invoke ========================";
    if ( jv.kind() == boost::json::kind::object ) {
        MSPeakInfoItem t;
        using namespace adportable::json;
        auto obj = jv.as_object();
        extract( obj, t.peak_index_         , "index"               );
        extract( obj, t.mass_               , "mass"                );
        extract( obj, t.area_               , "area"                );
        extract( obj, t.height_             , "height"              );
        extract( obj, t.peak_start_index_   , "peak_start_index"    );
        extract( obj, t.peak_end_index_     , "peak_end_index"      );
        extract( obj, t.base_height_        , "base_height"         );
        extract( obj, t.time_from_mass_     , "time_from_mass"      );
        extract( obj, t.time_from_time_     , "time_from_time"      );
        extract( obj, t.HH_left_mass_       , "HH_left_mass"        );
        extract( obj, t.HH_right_mass_      , "HH_right_mass"       );
        extract( obj, t.HH_left_time_       , "HH_left_time"        );
        extract( obj, t.HH_right_time_      , "HH_right_time"       );
        extract( obj, t.centroid_left_mass_ , "centroid_left_mass"  );
        extract( obj, t.centroid_right_mass_, "centroid_right_mass" );
        extract( obj, t.centroid_left_time_ , "centroid_left_time"  );
        extract( obj, t.centroid_right_time_, "centroid_right_time" );
        extract( obj, t.centroid_threshold_ , "centroid_threshold"  );
        extract( obj, t.is_visible_         , "is_visible"          );
        extract( obj, t.is_reference_       , "is_reference"        );
        int errc;
        extract( obj, t.formula_            , "formula",     errc   ); ADDEBUG() << errc;
        extract( obj, t.annotation_         , "annotation",  errc   ); ADDEBUG() << errc;
        return t;
    }
    return {};
}

void
MSPeakInfoItem::tag_invoke( boost::json::value& jv, adcontrols::MSPeakInfoItem const& t )
{
    jv = {
        { "index",                  t.peak_index_         }
        , { "mass",                 t.mass_               }
        , { "area",                 t.area_               }
        , { "height",               t.height_             }
        , { "peak_start_index",     t.peak_start_index_   }
        , { "peak_end_index",       t.peak_end_index_     }
        , { "base_height",          t.base_height_        }
        , { "time_from_mass",       t.time_from_mass_     }
        , { "time_from_time",       t.time_from_time_     }
        , { "HH_left_mass",         t.HH_left_mass_       }
        , { "HH_right_mass",        t.HH_right_mass_      }
        , { "HH_left_time",         t.HH_left_time_       }
        , { "HH_right_time",        t.HH_right_time_      }
        , { "centroid_left_mass",   t.centroid_left_mass_ }
        , { "centroid_right_mass",  t.centroid_right_mass_}
        , { "centroid_left_time",   t.centroid_left_time_ }
        , { "centroid_right_time",  t.centroid_right_time_}
        , { "centroid_threshold",   t.centroid_threshold_ }
        , { "is_visible",           t.is_visible_         }
        , { "is_reference",         t.is_reference_       }
        , { "mode",                 t.mode() ? *t.mode() : -1 }
        , { "formula",              t.formula_ }
        , { "annotation",           t.annotation_ }
    };
}

//////// JSON ////////
namespace adcontrols {
    void tag_invoke( const boost::json::value_from_tag
                     , boost::json::value& jv, const adcontrols::MSPeakInfoItem& t )
    {
        MSPeakInfoItem::tag_invoke( jv, t );
    }

    adcontrols::MSPeakInfoItem tag_invoke( const boost::json::value_to_tag< adcontrols::MSPeakInfoItem>&, const boost::json::value& jv )
    {
        return MSPeakInfoItem::tag_invoke( jv );
    }
}

/////// binary|xml  serializer ///////////
namespace adcontrols {

    template< typename T = MSPeakInfoItem >
    class MSPeakInfoItem::archiver {
    public:
        template<class Archive>
        void serialize( Archive& ar, T& _, const unsigned int version ) {
            if ( version >= 4 ) {
                ar  & BOOST_SERIALIZATION_NVP( _.peak_index_ )
                    & BOOST_SERIALIZATION_NVP( _.peak_start_index_ )
                    & BOOST_SERIALIZATION_NVP( _.peak_end_index_ )
                    & BOOST_SERIALIZATION_NVP( _.base_height_ )
                    & BOOST_SERIALIZATION_NVP( _.mass_ )
                    & BOOST_SERIALIZATION_NVP( _.area_ )
                    & BOOST_SERIALIZATION_NVP( _.height_ )
                    & BOOST_SERIALIZATION_NVP( _.time_from_mass_ )
                    & BOOST_SERIALIZATION_NVP( _.time_from_time_ )
                    & BOOST_SERIALIZATION_NVP( _.HH_left_mass_ )
                    & BOOST_SERIALIZATION_NVP( _.HH_right_mass_ )
                    & BOOST_SERIALIZATION_NVP( _.HH_left_time_ )
                    & BOOST_SERIALIZATION_NVP( _.HH_right_time_ )
                    & BOOST_SERIALIZATION_NVP( _.centroid_left_mass_ )
                    & BOOST_SERIALIZATION_NVP( _.centroid_right_mass_ )
                    & BOOST_SERIALIZATION_NVP( _.centroid_left_time_ )
                    & BOOST_SERIALIZATION_NVP( _.centroid_right_time_ )
                    & BOOST_SERIALIZATION_NVP( _.centroid_threshold_ )
                    ;
                ar & BOOST_SERIALIZATION_NVP( _.is_visible_ );
                ar & BOOST_SERIALIZATION_NVP( _.is_reference_ );
                ar & BOOST_SERIALIZATION_NVP( _.formula_ );
                ar & BOOST_SERIALIZATION_NVP( _.annotation_ ); // <-- chenged from std::wstring to std::string
                ar & BOOST_SERIALIZATION_NVP( _.mode_ );

            } else if ( version <= 3 ) {
                ar  & BOOST_SERIALIZATION_NVP( _.peak_index_ )
                    & BOOST_SERIALIZATION_NVP( _.peak_start_index_ )
                    & BOOST_SERIALIZATION_NVP( _.peak_end_index_ )
                    & BOOST_SERIALIZATION_NVP( _.base_height_ )
                    & BOOST_SERIALIZATION_NVP( _.mass_ )
                    & BOOST_SERIALIZATION_NVP( _.area_ )
                    & BOOST_SERIALIZATION_NVP( _.height_ )
                    & BOOST_SERIALIZATION_NVP( _.time_from_mass_ )
                    & BOOST_SERIALIZATION_NVP( _.time_from_time_ )
                    & BOOST_SERIALIZATION_NVP( _.HH_left_mass_ )
                    & BOOST_SERIALIZATION_NVP( _.HH_right_mass_ )
                    & BOOST_SERIALIZATION_NVP( _.HH_left_time_ )
                    & BOOST_SERIALIZATION_NVP( _.HH_right_time_ )
                    & BOOST_SERIALIZATION_NVP( _.centroid_left_mass_ )
                    & BOOST_SERIALIZATION_NVP( _.centroid_right_mass_ )
                    & BOOST_SERIALIZATION_NVP( _.centroid_left_time_ )
                    & BOOST_SERIALIZATION_NVP( _.centroid_right_time_ )
                    & BOOST_SERIALIZATION_NVP( _.centroid_threshold_ )
                    ;
                if ( version >= 2 ) {
                    std::wstring annotation;
                    ar & BOOST_SERIALIZATION_NVP( _.is_visible_ );
                    ar & BOOST_SERIALIZATION_NVP( _.is_reference_ );
                    ar & BOOST_SERIALIZATION_NVP( _.formula_ );
                    ar & BOOST_SERIALIZATION_NVP( annotation );
                    if ( Archive::is_loading::value )
                        _.annotation_ = utf::to_utf8( annotation );
                }
                if ( version >= 3 ) {
                    ar & BOOST_SERIALIZATION_NVP( _.mode_ );
                }
            }
        }
    };

    template<> void
    serialize( portable_binary_iarchive& ar, MSPeakInfoItem& t, const unsigned int version )
    {
#if __GNUC__
        MSPeakInfoItem::archiver<MSPeakInfoItem>().serialize( ar, t, version );
#else
        MSPeakInfoItem::archiver().serialize( ar, t, version );
#endif
    }

    template<> void
    serialize( portable_binary_oarchive& ar, MSPeakInfoItem& t, const unsigned int version )
    {
#if __GNUC__
        MSPeakInfoItem::archiver<MSPeakInfoItem>().serialize( ar, t, version );
#else
        MSPeakInfoItem::archiver().serialize( ar, t, version );
#endif
    }

    ///////// XML archive ////////
    template<> ADCONTROLSSHARED_EXPORT void
    serialize( boost::archive::xml_woarchive& ar, MSPeakInfoItem& t, const unsigned int version )
    {
#if __GNUC__
        MSPeakInfoItem::archiver<MSPeakInfoItem>().serialize( ar, t, version );
#else
        MSPeakInfoItem::archiver().serialize( ar, t, version );
#endif
    }

    template<> ADCONTROLSSHARED_EXPORT void
    serialize( boost::archive::xml_wiarchive& ar, MSPeakInfoItem& t, const unsigned int version )
    {
#if __GNUC__
        MSPeakInfoItem::archiver<MSPeakInfoItem>().serialize( ar, t, version );
#else
        MSPeakInfoItem::archiver().serialize( ar, t, version );
#endif
    }

}
