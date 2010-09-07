//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "qtxml.h"
#include <QFile>

using namespace xmlwrapper;
using namespace xmlwrapper::qtxml;

XMLDocument::~XMLDocument()
{
}

XMLDocument::XMLDocument()
{
}

XMLDocument::XMLDocument( const XMLDocument& t ) : impl_( t.impl_ )
{
}
    
bool
XMLDocument::save( const xmlstring& filename ) const
{
    (void)filename;
    return false;
}

bool
XMLDocument::load( const xmlstring& filename )
{
    QFile file( filename );
    if ( ! file.open( QIODevice::ReadOnly ) )
        return false;
    return impl_.setContent( &file );
}

bool
XMLDocument::loadXML( const xmlstring& xml )
{
    return impl_.setContent( xml );
}
    
xmlstring
XMLDocument::toString() const
{
    return impl_.toString();
}

XMLProcessingInstruction
XMLDocument::createProcessingInstruction( const xmlstring& target, const xmlstring& data)
{
    return impl_.createProcessingInstruction( target, data );
}

XMLElement
XMLDocument::documentElement()
{
    return impl_.documentElement();
}

XMLElement
XMLDocument::createElement( const xmlstring& name )
{
    return impl_.createElement( name );
}

XMLText
XMLDocument::createTextNode( const xmlstring& name )
{
    return impl_.createTextNode( name );
}
    
XMLComment
XMLDocument::createComment( const xmlstring& name )
{
    return impl_.createComment( name );
}

XMLAttr
XMLDocument::createAttribute( const xmlstring& name )
{
    return impl_.createAttribute( name );
}

XMLNode
XMLDocument::importNode(XMLNode & node, bool deep )
{
    return impl_.importNode( node, deep );
}

///////////////////////////////

XMLNode::~XMLNode()
{
}

XMLNode::XMLNode()
{
}

XMLNode::XMLNode( const XMLNode& t ) : impl_(t.impl_)
{
}

XMLNode::XMLNode( const xmldomnode_type& t ) : impl_(t)
{
}

XMLNode::operator qtxml::xmldomnode_type & ()
{
    return impl_;
}

xmlstring
XMLNode::nodeName() const
{
    return impl_.nodeName();
}

////////////////////////////////////////////////
XMLProcessingInstruction::~XMLProcessingInstruction()
{
}

XMLProcessingInstruction::XMLProcessingInstruction()
{
}

XMLProcessingInstruction::XMLProcessingInstruction( const XMLNode& t ) : XMLNode(t)
{
}

XMLProcessingInstruction::XMLProcessingInstruction( const QDomProcessingInstruction& t ) : XMLNode( static_cast<const QDomNode&>(t) )
{
}

/////////////////////////////////////////////////////

XMLElement::~XMLElement()
{
}

XMLElement::XMLElement()
{
}

XMLElement::XMLElement( const XMLNode& t ) : XMLNode(t)
{
}

XMLElement::XMLElement( const QDomElement& t ) : XMLNode( static_cast<const QDomNode&>(t) )
{
}

/////////////////////////////////////////////////////

XMLText::~XMLText()
{
}

XMLText::XMLText()
{
}

XMLText::XMLText( const XMLNode& t ) : XMLNode(t)
{
}

XMLText::XMLText( const QDomText& t ) : XMLNode( static_cast<const QDomNode&>(t) )
{
}

/////////////////////////////////////////////////////

XMLComment::~XMLComment()
{
}

XMLComment::XMLComment()
{
}

XMLComment::XMLComment( const QDomComment& t ) : XMLNode( static_cast<const QDomNode&>(t) )
{
}

/////////////////////////////////////////////////////////////////
XMLAttr::~XMLAttr()
{
}

XMLAttr::XMLAttr()
{
}

XMLAttr::XMLAttr( const QDomAttr& t ) : XMLNode( static_cast<const QDomNode&>(t) )
{
}
