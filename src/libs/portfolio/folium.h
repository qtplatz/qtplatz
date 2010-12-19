// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "portfolio_global.h"
#include <boost/any.hpp>
#include "node.h"

namespace adcontrols {
    class Chromatogram;
    class MassSpectrum;
}

namespace portfolio {

    class PORTFOLIOSHARED_EXPORT Folium : public internal::Node {
    public:
        ~Folium();
        Folium();
        Folium( const Folium& );
        Folium( xmlNode& );
        //Folium( xmlNode&, const::boost::any& a );
        //Folium( const adcontrols::Chromatogram& );
        //Folium( const adcontrols::MassSpectrum& );

    private:
        boost::any any_;
    };

    typedef std::vector< Folium > Folio;

}


