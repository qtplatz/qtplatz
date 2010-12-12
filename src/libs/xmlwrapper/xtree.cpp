//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "xtree.h"
#include <string>

#if 0
XMLNode
xtree_node::appendChild(XMLDocument& dom, XMLElement& elm, const std::wstring& name, int& nLevel)
{
    XMLNode child = elm.selectSingleNode(L"*");
    if ( ! child ) {
        XMLNode::appendText(dom, elm, L"\n");
        if ( nLevel == 0 ) {
            XMLNode parent = elm.selectSingleNode( L".." );
            while ( parent ) {
                parent = parent.selectSingleNode( L".." );
                ++nLevel;
            }
        }
    }
    std::wstring tabs;
    for ( int i = 0; i < nLevel; ++i )
        tabs += L"\t";

    XMLNode node = dom.createElement( name );
    XMLNode::appendText(dom, elm, tabs);
    elm.appendChild( node );
    XMLNode::appendText(dom, elm, L"\n");
    return node;
}

std::wstring
xtree_node::newGUID()
{
    GUID guid(GUID_NULL);
    ::CoCreateGuid(&guid);
    std::wstring guidStr = CTypeLib::StringFromCLSID(guid);
    return guidStr;
}

std::wstring
xtree_node::getFullyQualifiedName( const XMLElement& e )
{
    std::wstring fqn = e.getNodeName();
    XMLNode parent = e.selectSingleNode( L".." );
    while ( parent ) {
        fqn = parent.getNodeName() + L"/" + fqn;
        parent = parent.selectSingleNode( L".." );
    }
    return L"/" + fqn;
}
#endif
