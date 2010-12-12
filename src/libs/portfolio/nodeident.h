// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <string>
#include <xmlwrapper/msxml.h>

namespace portfolio {

    typedef xmlwrapper::msxml::XMLElement  xmlElement;
    typedef xmlwrapper::msxml::XMLDocument xmlDocument;
    typedef xmlwrapper::msxml::XMLNode     xmlNode;
    typedef xmlwrapper::msxml::XMLNodeList xmlNodeList;

    namespace internal {

        class NodeIdent {
        public:
            NodeIdent();
            NodeIdent( const NodeIdent& );
            NodeIdent( const xmlElement& );

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

        protected:
            xmlElement node_;
        };

    }
}

