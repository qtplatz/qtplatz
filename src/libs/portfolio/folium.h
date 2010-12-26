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

    namespace internal {
        class PortfolioImpl;
    }

    class PORTFOLIOSHARED_EXPORT Folium : public internal::Node {
    public:
        ~Folium();
        Folium();
        Folium( const Folium& );
        Folium( xmlNode&, internal::PortfolioImpl * impl );

        std::wstring path() const;
        std::wstring dataType() const;

        bool empty() const;
        void operator = ( boost::any& );
        operator boost::any& ();
    };

    typedef std::vector< Folium > Folio;

}


