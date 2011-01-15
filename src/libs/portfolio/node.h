// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/

#pragma once

#include "portfolio_global.h"
#include <string>
#include <xmlwrapper/msxml.h>

namespace portfolio {

    typedef xmlwrapper::msxml::XMLElement  xmlElement;
    typedef xmlwrapper::msxml::XMLDocument xmlDocument;
    typedef xmlwrapper::msxml::XMLNode     xmlNode;
    typedef xmlwrapper::msxml::XMLNodeList xmlNodeList;

    namespace internal {

        class PortfolioImpl;

        class PORTFOLIOSHARED_EXPORT Node {
        public:
            Node();
            Node( const Node& );
        protected:
            Node( const xmlElement&, PortfolioImpl* impl );

        public:
            std::wstring name() const;
            void name( const std::wstring& name );

            std::wstring id() const;
            void id( const std::wstring& );

            bool isFolder() const;
            void isFolder( bool );

            std::wstring dataClass() const;
            void dataClass( const std::wstring& );

            std::wstring attribute( const std::wstring& ) const;
            void setAttribute( const std::wstring& key, const std::wstring& value );

            xmlNodeList selectNodes( const std::wstring& query );

        protected:
            xmlElement addFolder( const std::wstring& name, PortfolioImpl* );
            xmlElement addFolium( const std::wstring& name );
            xmlElement addAttachment( const std::wstring& name );

        protected:
# pragma warning (disable: 4251)
            xmlElement node_;
# pragma warning (default: 4251)
            PortfolioImpl* impl_;
        };

    }
}

