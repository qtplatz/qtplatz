// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include "qtxml.hpp"
#include <QFile>
#include <qtwrapper/qstring.hpp>

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

bool
XMLDocument::loadXML( const std::wstring& xml )
{
    return impl_.setContent( qtwrapper::qstring::copy(xml) );
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
