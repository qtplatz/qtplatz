// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "portfolio_global.h"
#include <vector>
#include "nodeident.h"

namespace portfolio {

    class Folium;

    // folder can be directory, or data (folio)

    class PORTFOLIOSHARED_EXPORT Folder : public internal::NodeIdent {
    public:
        ~Folder();
        Folder();
        Folder( const Folder& );

    private:
        std::vector< Folium > folio_; // Chromatogram, Spectrum etc.
    };

}

