// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "portfolio_global.h"
#include <vector>
#include "node.h"

namespace portfolio {

    class Folium;

    // folder can be directory, or data (folio)

    class PORTFOLIOSHARED_EXPORT Folder : public internal::Node {
    public:
        ~Folder();
        Folder();
        Folder( const xmlNode& );
        Folder( const Folder& );

        std::vector< Folder > folders();
        std::vector< Folium > folio();
        Folium selectSingleFolium( const std::wstring& );
    };

}

