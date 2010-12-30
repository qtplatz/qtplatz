//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

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
/*
    CY	fontSize;
    fontSize.Hi	= 0;
    fontSize.Lo = 80000;
    CComQIPtr<IFont> piFont = piTrace->Font;
    piFont->put_Size( fontSize );
    piFont->put_Bold( FALSE );
*/
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
Annotations::operator [] ( int idx )
{
    return Annotation( pi_->Item[ idx ] );
}