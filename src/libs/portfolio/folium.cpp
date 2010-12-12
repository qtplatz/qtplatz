//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "folium.h"
#include <adcontrols/Chromatogram.h>
#include <adcontrols/MassSpectrum.h>

using namespace portfolio;

Folium::~Folium()
{
}

Folium::Folium()
{
}

Folium::Folium( const Folium& t ) : any_( t.any_ )
{
}

Folium::Folium( const adcontrols::Chromatogram& t ) : any_( boost::any(t) )
{
}

Folium::Folium( const adcontrols::MassSpectrum& t ) : any_( boost::any(t) )
{
}
