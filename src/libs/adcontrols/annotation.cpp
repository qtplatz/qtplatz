/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

using namespace adcontrols;

annotation::annotation() : format_( dataText )
                         , index_( -1 )
                         , priority_( 0 )
                         , x_( 0 )
                         , y_( 0 )
                         , w_( 0 )
                         , h_( 0 )
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
{
}

annotation::annotation( const std::wstring& text
                        , double x
                        , double y
                        , int idx
                        , int priority
                        , DataFormat typ ) : format_( typ )
                                           , index_( idx )
                                           , x_( x ), y_( y )
                                           , priority_( priority )
										   , text_( adportable::utf::to_utf8( text ) )
                                           , w_( 0 )
                                           , h_( 0 )
{
}

annotation::annotation( const std::string& text
                        , double x
                        , double y
                        , int idx
                        , int priority
                        , DataFormat typ ) : format_( typ )
                                           , index_( idx )
                                           , x_( x ), y_( y )
                                           , priority_( priority )
										   , text_( text )
                                           , w_( 0 )
                                           , h_( 0 )
{
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

