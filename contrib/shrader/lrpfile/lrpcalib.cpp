/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "lrpcalib.hpp"
#include <adportable/debug.hpp>
#include <boost/json.hpp>
#include <cstddef>
#include <istream>

namespace shrader {
    namespace detail {

#pragma pack(1)
        struct CAL {
            int32_t m;     // manual: "Number indices in segment"; meaning uncertain
            float i;       // manual: "Number turns"; clearly not literal laps in some files
            double coeffa; // manual: "Segment start time"; uncertain
            double coeffb; // manual: "MT Conversion factor"; likely calibration-related
        };

        struct calib {
            int32_t flags;
            CAL cal[ 10 ];
            char type[ 8 ];
            char dummy[ 4 ];
        };
#pragma pack()

    }
}

using namespace shrader;

lrpcalib::~lrpcalib()
{
}

lrpcalib::lrpcalib() : loaded_( false )
{
}

lrpcalib::lrpcalib( const lrpcalib& t ) : loaded_( t.loaded_ )
                                        , data_( t.data_ )
{
}

bool
lrpcalib::load( std::istream& in, size_t fsize )
{
    loaded_ = false;
    if ( ( fsize - in.tellg() ) >= data_size ) {
        in.read( data_.data(), data_.size() );
        auto d = reinterpret_cast< const detail::calib * >( data_.data() );
        if ( not in.fail() && d->flags == record_type_code )
            loaded_ = true;
    }
    return loaded_;
}

int32_t
lrpcalib::flags() const
{
    return *reinterpret_cast<const int32_t *>(data_.data() + offsetof( detail::calib, flags ));
}

cal_data
lrpcalib:: cal_data( size_t idx ) const
{
    if ( idx < cal_size ) {
        auto p = reinterpret_cast<const detail::CAL *>(data_.data() + offsetof( detail::calib, cal ));
        return { p[idx].m, p[idx].i, p[idx].coeffa, p[idx].coeffb };
    }
    return {};
}

std::string
lrpcalib::type() const
{
    return std::string( data_.data() + offsetof( detail::calib, type ), 8 );
}

namespace shrader {

    namespace detail {
        void
        tag_invoke( const boost::json::value_from_tag, boost::json::value& jv, const CAL& t )
        {
            jv = {{ "m", t.m }
                  , { "i", t.i }
                  , { "coeffa", t.coeffa }
                  , { "coeffb", t.coeffb }
            };
        }
    } // detail

    void
    tag_invoke( const boost::json::value_from_tag, boost::json::value& jv, const lrpcalib& t )
    {
        auto p = reinterpret_cast<const detail::calib *>(t.data_.data());

        jv = {{ "calib"
                    , {{ "flags", p->flags }
                       , { "cal", boost::json::value_from( p->cal ) }
                       , { "type", p->type }
                       , { "dummy", p->dummy }
                }
            }};
    }

    lrpcalib
    tag_invoke( const boost::json::value_to_tag< lrpcalib >&, const boost::json::value& jv )
    {
        return {};
    }

}
