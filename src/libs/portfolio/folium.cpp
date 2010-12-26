//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "folium.h"
#include "portfolioimpl.h"
#include <adcontrols/Chromatogram.h>
#include <adcontrols/MassSpectrum.h>

using namespace portfolio;

Folium::~Folium()
{
}

Folium::Folium()
{
}

Folium::Folium( const Folium& t ) : Node( t ) 
{
}

Folium::Folium( xmlNode& n, internal::PortfolioImpl * impl ) : Node( n, impl )
{
}

std::wstring
Folium::path() const
{
    return attribute( L"path" );
}

std::wstring
Folium::dataType() const
{
    return attribute( L"dataType" );
}

bool
Folium::empty() const
{
    if ( impl_ ) {
        boost::any& data = impl_->find( id() );
        return data.empty();
    }
    return true;
}

void
Folium::operator = ( boost::any& any )
{
    if ( impl_ )
        impl_->assign( id(), any );
}

Folium::operator boost::any & ()
{
    if ( impl_ )
        return impl_->find( id() );
    static boost::any temp;
    return temp;
}

/*
Folium::Folium( const boost::any& t ) : any_( t )
{
}

Folium::Folium( const adcontrols::Chromatogram& t ) : any_( boost::any(t) )
{
}

Folium::Folium( const adcontrols::MassSpectrum& t ) : any_( boost::any(t) )
{
}
*/
