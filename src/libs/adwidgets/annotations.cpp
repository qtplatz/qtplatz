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

#include "annotations.h"
#include "annotation.h"
#include "import_sagraphics.h"

using namespace adwidgets::ui;

Annotations::~Annotations()
{
    if ( pi_ )
        pi_->Release();
}

Annotations::Annotations( SAGRAPHICSLib::ISADPAnnotations * pi ) : pi_(pi)
{
    pi_->AddRef();
}

Annotations::Annotations( const Annotations& t )
{
    if ( t.pi_ )
        t.pi_->AddRef(); // AddRef first, in order to avoid unexpected release when self assignment happens
    if ( pi_ )
        pi_->Release();
    pi_ = t.pi_;
}

void
Annotations::visible( bool f )
{
    pi_->Visible = internal::variant_bool::to_variant( f );
}

bool
Annotations::visible() const
{
    return internal::variant_bool::to_native( pi_->Visible );
}

bool
Annotations::annotateX() const
{
    return internal::variant_bool::to_native( pi_->AnnotateX );
}

void
Annotations::annotateX( bool f )
{
    pi_->AnnotateX = internal::variant_bool::to_variant( f );
}

bool
Annotations::annotateY() const
{
    return internal::variant_bool::to_native( pi_->AnnotateY );
}

void
Annotations::annotateY( bool f )
{
    pi_->AnnotateY = internal::variant_bool::to_variant( f );
}

int
Annotations::decimalsX() const
{
    return pi_->XDecimals;
}

void
Annotations::decimalsX( int value )
{
    pi_->XDecimals = value;
}

int
Annotations::decimalsY() const
{
    return pi_->YDecimals;
}

void
Annotations::decimalsY( int value )
{
    pi_->YDecimals = value;
}

double
Annotations::textAngle() const
{
    return pi_->TextAngle;
}

void
Annotations::textAngle( double value )
{
    pi_->TextAngle = static_cast<float>(value);
}

bool
Annotations::centreHorizontal() const
{
    return internal::variant_bool::to_native( pi_->CenterHorizontal );
}

bool
Annotations::centreVerticl() const
{
    return internal::variant_bool::to_native( pi_->CenterVertical );
}

void
Annotations::centreHorizontal( bool value )
{
    pi_->CenterHorizontal = internal::variant_bool::to_variant( value );
}

void
Annotations::centreVertical( bool value )
{
    pi_->CenterVertical = internal::variant_bool::to_variant( value );
}

Annotation
Annotations::add()
{
   return Annotation( pi_->Add() );
}

void
Annotations::remove( int idx )
{
    pi_->Remove( idx );
}

Annotation
Annotations::operator [] ( int idx ) // 0 origin
{
    return Annotation( pi_->Item[ idx + 1 ] );
}