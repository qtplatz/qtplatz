// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "portfolio_global.h"
#include <boost/smart_ptr.hpp>
#include <vector>

namespace portfolio {

    namespace internal {
        class PortfolioImpl;
    }

    class Folium;
    class Folder;

    class PORTFOLIOSHARED_EXPORT Portfolio {
    public:
        ~Portfolio();
        Portfolio();
        Portfolio( const Portfolio& );
        Portfolio( const std::wstring& xml );

        std::vector<Folder> folders();
     
    private:
        boost::shared_ptr< internal::PortfolioImpl > impl_;
  };

}
