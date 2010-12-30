/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#include "font.h"
#include "import_sagraphics.h"

using namespace adwidgets;

ui::Font::~Font()
{
    if ( pi_ )
        pi_->Release();
}

ui::Font::Font( IDispatch * pi ) : pi_(pi)
{
    if ( pi_ )
        pi_->AddRef();
}

ui::Font::Font( const Font& t )
{
    if ( t.pi_ )
        t.pi_->AddRef(); // AddRef first, in order to avoid unexpected release when self assignment happens
    if ( pi_ )
        pi_->Release();
    pi_ = t.pi_;
}

bool
ui::Font::bold() const
{
    IFontPtr ifont = pi_;
    BOOL result;
    if ( ifont && ( ifont->get_Bold( &result ) == S_OK ) )
        return result ? true : false;
    return false;
}

void
ui::Font::bold( bool value ) 
{
    IFontPtr ifont = pi_;
    if ( ifont )
        ifont->put_Italic( value ? TRUE : FALSE );
}

short
ui::Font::charset() const
{
    IFontPtr ifont = pi_;
    short result;
    if ( ifont && ( ifont->get_Charset( &result ) == S_OK ) )
        return result;
    return 0;
}

// HFONT hFont() const;

bool
ui::Font::italic() const
{
    IFontPtr ifont = pi_;
    BOOL result;
    if ( ifont && ( ifont->get_Italic( &result ) == S_OK ) )
        return result ? true : false;
    return false;
}

void
ui::Font::italic( bool value )
{
    IFontPtr ifont = pi_;
    if ( ifont )
        ifont->put_Italic( value ? TRUE : FALSE );
}

std::wstring
ui::Font::name() const
{
    IFontPtr ifont = pi_;
    CComBSTR result;
    if ( ifont && ( ifont->get_Name( &result ) == S_OK ) )
        return std::wstring( result );
    return L"";
}

long long
ui::Font::size() const
{
    IFontPtr ifont = pi_;
    CY result;
    if ( ifont && ( ifont->get_Size( &result ) == S_OK ) )
        return result.int64;
    return 0;
}

void
ui::Font::size( long long value )
{
    IFontPtr ifont = pi_;
    CY fontSize;
    fontSize.int64 = value;
    if ( ifont )
        ifont->put_Size( fontSize );
}

bool
ui::Font::strikethrough() const
{
    IFontPtr ifont = pi_;
    BOOL result;
    if ( ifont && ( ifont->get_Strikethrough( &result ) == S_OK ) )
        return result ? true : false;
    return false;
}

bool
ui::Font::underline() const
{
    IFontPtr ifont = pi_;
    BOOL result;
    if ( ifont && ( ifont->get_Underline( &result ) == S_OK ) )
        return result ? true : false;
    return false;
}

short
ui::Font::weight() const
{
    IFontPtr ifont = pi_;
    short result;
    if ( ifont && ( ifont->get_Weight( &result ) == S_OK ) )
        return result;
    return 0;
}
