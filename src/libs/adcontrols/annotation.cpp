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

#include "annotation.hpp"
#include <adportable/utf.hpp>
#include <adportable/json/extract.hpp>
#include <adportable/json_helper.hpp>
// #include <boost/property_tree/json_parser.hpp>
#include <boost/json.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>

using namespace adcontrols;

annotation::~annotation()
{
}

annotation::annotation() : format_( dataText )
                         , index_( -1 )
                         , priority_( 0 )
                         , x_( 0 )
                         , y_( 0 )
                         , w_( 0 )
                         , h_( 0 )
                         , flags_( 0 )
{
}

annotation::annotation( const annotation& t ) : format_( t.format_ )
                                              , index_( t.index_ )
                                              , priority_( t.priority_ )
                                              , text_( t.text_ )
                                              , x_( t.x_ )
                                              , y_( t.y_ )
                                              , w_( t.w_ )
                                              , h_( t.h_ )
                                              , flags_( t.flags_ )
{
}

annotation::annotation( const std::wstring& text
                        , double x
                        , double y
                        , int idx
                        , int priority
                        , DataFormat typ
                        , DataFlag flg ) : format_( typ )
                                         , index_( idx )
                                         , priority_( priority )
                                         , text_( adportable::utf::to_utf8( text ) )
                                         , x_( x ), y_( y )
                                         , w_( 0 )
                                         , h_( 0 )
                                         , flags_( flg )
{
}

annotation::annotation( const std::string& text
                        , double x
                        , double y
                        , int idx
                        , int priority
                        , DataFormat typ
                        , DataFlag flg ) : format_( typ )
                                         , index_( idx )
                                         , priority_( priority )
                                         , text_( text )
                                         , x_( x ), y_( y )
                                         , w_( 0 )
                                         , h_( 0 )
                                         , flags_( flg )
{
}

annotation::annotation( boost::json::value&& value
                        , double x
                        , double y
                        , int idx
                        , int priority
                        , DataFlag flg ) : format_( dataJSON )
                                         , index_( idx )
                                         , priority_( priority )
                                         , x_( x ), y_( y )
                                         , w_( 0 )
                                         , h_( 0 )
                                         , flags_( flg )
{
    text_ = boost::json::serialize( value );
}

boost::optional< std::string >
annotation::json() const
{
    if ( !text_.empty() && format_ == dataJSON )
        return text_;
    return boost::none;
}

boost::json::value
annotation::value() const
{
    return adportable::json_helper::parse( text_ );
}

const std::string&
annotation::text() const
{
    return text_;
}

void
annotation::text( const std::string& text, DataFormat format )
{
    text_ = text;
    format_ = format;
}

void
annotation::text( const std::wstring& text, DataFormat format )
{
    text_ = adportable::utf::to_utf8( text );
    format_ = format;
}

int
annotation::index() const
{
    return index_;
}

void
annotation::index( int idx )
{
    index_ = idx;
}


enum annotation::DataFormat
annotation::dataFormat() const
{
    return format_;
}

void
annotation::dataFormat( enum DataFormat t )
{
    format_ = t;
}

int
annotation::priority() const
{
    return priority_;
}

void
annotation::priority( int pri )
{
    priority_ = pri;
}


double
annotation::x() const
{
    return x_;
}

double
annotation::y() const
{
    return y_;
}

double
annotation::width() const
{
    return w_;
}

double
annotation::height() const
{
    return h_;
}

void
annotation::rect( double x, double y, double width, double height )
{
    x_ = x;
    y_ = y;
    w_ = width;
    h_ = height;
}

void
annotation::x( double x )
{
    x_ = x;
}

void
annotation::y( double y )
{
    y_ = y;
}

uint32_t
annotation::flags() const
{
    return flags_;
}

void
annotation::setFlags( uint32_t f )
{
    flags_ = f;
}

namespace adcontrols {
    annotation::reference_molecule::reference_molecule() : exact_mass_(0), mass_(0) {
    }

    annotation::reference_molecule::reference_molecule( const std::string& display_name
                                                        , const std::string& formula
                                                        , const std::string& adduct
                                                        , double exact_mass
                                                        , double mass
                                                        , const boost::json::value& jv )
        : display_name_( display_name )
        , formula_( formula )
        , adduct_( adduct )
        , exact_mass_( exact_mass )
        , mass_( mass )
        , origin_( jv ) {
    }

    annotation::reference_molecule::reference_molecule( const reference_molecule& t )
        : display_name_( t.display_name_ )
        , formula_( t.formula_ )
        , adduct_( t.adduct_ )
        , exact_mass_( t.exact_mass_ )
        , mass_( t.mass_ )
        , origin_( t.origin_ ) {
    }


    void
    tag_invoke( const boost::json::value_from_tag, boost::json::value& jv, const annotation::peak& t )
    {
        jv = {{ "peak"
                , {{ "mode", t.mode }
                    , { "mass", t.mass }}
            }};
    }

    annotation::peak tag_invoke( const boost::json::value_to_tag< annotation::peak >&, const boost::json::value& jv )
    {
        annotation::peak t;
        if ( jv.is_object() ) {
            auto obj = jv.as_object();
            auto sobj = obj.at( "peak" ).as_object();
            adportable::json::extract( sobj, t.mode, "mode" );
            adportable::json::extract( sobj, t.mass, "mass" );
        }
        return t;
    }

    void
    tag_invoke( const boost::json::value_from_tag, boost::json::value& jv, const annotation::reference_molecule& t )
    {
        jv = {{ "refernce_molecule"
                    , {{ "display_name", t.display_name_ }
                       , { "formula",       t.formula_ }
                       , { "adduct",        t.adduct_ }
                       , { "exact_mass",    t.exact_mass_ }
                       , { "mass",          t.mass_ }
                       , { "origin",        t.origin_ }
                }
            }};
    }

    annotation::reference_molecule
    tag_invoke( const boost::json::value_to_tag< annotation::reference_molecule >&, const boost::json::value& jv )
    {
        annotation::reference_molecule t;
        if ( jv.is_object() ) {
            auto obj = jv.as_object();
            auto sobj = obj.at( "reference_molecule" ).as_object();
            adportable::json::extract( sobj, t.display_name_, "display_name" );
            adportable::json::extract( sobj, t.formula_,      "formula" );
            adportable::json::extract( sobj, t.adduct_,       "adduct" );
            adportable::json::extract( sobj, t.exact_mass_,   "exact_mass" );
            adportable::json::extract( sobj, t.mass_,         "mass" );
            adportable::json::extract( sobj, t.origin_,       "origin" );

        }
        return t;
    }

}
