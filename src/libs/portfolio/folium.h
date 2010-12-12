// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "portfolio_global.h"
#include <boost/any.hpp>
#include "nodeident.h"

namespace adcontrols {
    class Chromatogram;
    class MassSpectrum;
}

namespace portfolio {

    class PORTFOLIOSHARED_EXPORT Folium : public internal::NodeIdent {
    public:
        ~Folium();
        Folium();
        Folium( const Folium& );

        Folium( const::boost::any& a );
        Folium( const adcontrols::Chromatogram& );
        Folium( const adcontrols::MassSpectrum& );

    private:
        boost::any any_;
    };

}


