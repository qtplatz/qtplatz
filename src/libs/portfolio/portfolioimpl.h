// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <boost/any.hpp>
#include <map>
#include <xmlwrapper/msxml.h>
#include "node.h"

namespace portfolio {

    class Folium;
    class Folder;

    namespace internal {

        // Portfolio is a root folder

        class PortfolioImpl : public Node {
        public:
            PortfolioImpl();
            PortfolioImpl( const PortfolioImpl& );
            PortfolioImpl( const std::wstring& xml );
            operator bool () const;
            const std::wstring fullpath() const;
            std::vector<Folder> selectFolders( const std::wstring& );

        private:
            bool isXMLLoaded_;
            std::map< std::wstring, boost::any > db_;
            xmlDocument doc_;
        };
    }
}


