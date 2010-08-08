// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <string>
#include <boost/smart_ptr.hpp>
#include <QtXml/QtXml>
#include <QtXmlPatterns/QXmlQuery>
#include <QtXmlPatterns/QXmlResultItems>

class QDomDocument;
class QDomComment;
class QDomElement;
class QDomAtrr;
class QDomText;
class QDomNodeList;
class QDomNode;
class QDomProcessingInstruction;

namespace qtwrapper {

    typedef QDomNode xmldomnode_type;
    typedef QDomDocument xmldomdocument_type;
    typedef QDomElement xmlelement_type;
    typedef QDomText xmltext_type;
    typedef QDomComment xmlcomment_type;
    typedef QDomAttr xmlattr_type;
    typedef QString xmlstring;

    class XMLDocument;
    class XMLComment;
    class XMLElement;
    class XMLAttr;
    class XMLText;
    class XMLNodeList;
    class XMLProcessingInstruction;

    class XMLNode {
    private:
        xmldomnode_type impl_;
    public:
        virtual ~XMLNode();
        XMLNode();
        XMLNode( const XMLNode & );
        XMLNode( const xmldomnode_type& );

        operator xmldomnode_type& ();
    
        // XMLDocumentType
        xmlstring nodeName() const;
        xmlstring getTextValue() const;
        void setTextValue( const xmlstring& );
        const XMLAttr getAttributeNode( const xmlstring& ) const;
    
        std::wstring getAttribute( const xmlstring& ) const;
        XMLNodeList getChildNodes() const;
        XMLNode appendChild( XMLNode& );
        XMLNode removeChild( XMLNode& );
        XMLNode cloneNode(bool deep) const;
    
        // The Xerces/Xalan implementation of these functions is NOT thread-safe!
        XMLNodeList selectNodes(const xmlstring& query) const;
        XMLNode selectSingleNode(const xmlstring& query) const;
        std::wstring transformNode( XMLNode& );
    
        static void appendText( XMLDocument& dom, XMLNode& node, const xmlstring& text );
    };
  
    class XMLDocument : public XMLNode {
    private:
        xmldomdocument_type impl_;

    public:
        virtual ~XMLDocument();
        XMLDocument();
        XMLDocument( const XMLDocument& );

        bool save( const xmlstring& filename ) const;
        bool load( const xmlstring& filename );
        bool loadXML( const xmlstring& xml );
    
        xmlstring toString() const;

        XMLProcessingInstruction createProcessingInstruction( const xmlstring& target, const xmlstring& data);
        XMLElement documentElement();
        XMLElement createElement( const xmlstring& );
        XMLText createTextNode( const xmlstring& );
    
        XMLComment createComment( const xmlstring& );
        XMLAttr createAttribute( const xmlstring& );
        XMLNode importNode(XMLNode & node, bool deep );
    };
  
    class XMLElement : public XMLNode {
    public:
        virtual ~XMLElement();
        XMLElement();
        XMLElement( const XMLNode & );
        XMLElement( const xmlelement_type& );
    
        void setAttribute(const std::wstring& name, const std::wstring& value);
        // std::wstring getAttribute( const std::wstring& name ) const;
    };
  
    class XMLProcessingInstruction : public XMLNode {
    public:
        virtual ~XMLProcessingInstruction();
        XMLProcessingInstruction();
        XMLProcessingInstruction( const XMLNode& );
        XMLProcessingInstruction( const QDomProcessingInstruction& );
    };
  
    class XMLAttr : public XMLNode {
    public:
        virtual ~XMLAttr();
        XMLAttr();
        XMLAttr( const XMLNode& );
        XMLAttr( const xmlattr_type& );
    };
  
    class XMLText : public XMLNode {
    public:
        virtual ~XMLText();
        XMLText();
        XMLText( const XMLNode& );
        XMLText( const xmltext_type& );
    };
  
    class XMLComment : public XMLNode {
    public:
        virtual ~XMLComment();
        XMLComment();
        XMLComment( const XMLComment& );
        XMLComment( const xmlcomment_type& );
    };
  
    class XMLNodeList {
    protected:

    public:
        virtual ~XMLNodeList();
        XMLNodeList();
        XMLNodeList( const XMLNodeList & other );
    
        size_t size() const;
        XMLNode operator [] (int idx) const;
    };

}
