// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <vector>
#include <string>
#include <boost/smart_ptr.hpp>
#include <boost/intrusive_ptr.hpp>

typedef wchar_t XMLCh;

#if defined WIN32

struct IDispatch;

namespace MSXML2 {
   struct IXMLDOMNode;
   struct IXMLDOMNodeList;
};

typedef MSXML2::IXMLDOMNode xmldomnode_type;
typedef MSXML2::IXMLDOMNodeList xmldomnodelist_type;

void intrusive_ptr_add_ref( xmldomnode_type * ptr );
void intrusive_ptr_release( xmldomnode_type * ptr );

void intrusive_ptr_add_ref( xmldomnodelist_type * ptr );
void intrusive_ptr_release( xmldomnodelist_type * ptr );

#endif

namespace xmlwrapper {

  namespace msxml {
    
    class XMLDocument;
    class XMLComment;
    class XMLElement;
    class XMLAttr;
    class XMLText;
    class XMLNodeList;
    class XMLProcessingInstruction;
    
    class XMLNode {
    protected:
      
      boost::intrusive_ptr< xmldomnode_type > pImpl_;
      
    public:
      virtual ~XMLNode();
      XMLNode();
      XMLNode( const XMLNode & );
      
      XMLNode( xmldomnode_type * );
      XMLNode( IDispatch * );
      
      operator xmldomnode_type * ();
      operator bool() const;
      
      // XMLDocumentType
      std::wstring nodeName() const;
      std::wstring textValue() const;
      void textValue( const std::wstring& );
      const XMLAttr attributeNode( const std::wstring& ) const;
      
      std::wstring attribute( const std::wstring& ) const;
      XMLNodeList childNodes() const;
      XMLNode appendChild( XMLNode& );
      XMLNode removeChild( XMLNode& );
      XMLNode cloneNode(bool deep) const;
      
      // The Xerces/Xalan implementation of these functions is NOT thread-safe!
      XMLNodeList selectNodes(const std::wstring& query) const;
      XMLNode selectSingleNode(const std::wstring& query) const;
      std::wstring transformNode( XMLNode& );
      
      static void appendText( XMLDocument& dom, XMLNode& node, const std::wstring& text );
    };
    
    class XMLDocument : public XMLNode {
    public:
      virtual ~XMLDocument();
      XMLDocument();
      XMLDocument( const XMLDocument& );
      
      explicit XMLDocument( IDispatch * );
      static std::wstring toString( const XMLElement& );
      
      bool save( const std::wstring& filename ) const;
      bool load( const std::wstring& filename );
      bool loadXML( const std::wstring& xml );
      
      bool xml(std::wstring& xml) const;
	  std::wstring toString() const;
      
      XMLProcessingInstruction createProcessingInstruction( const std::wstring& target
							    , const std::wstring& data);
      XMLElement documentElement();
      XMLElement createElement( const std::wstring& );
      XMLText createTextNode( const std::wstring& );
      
      XMLComment createComment(const std::wstring& );
      XMLAttr createAttribute(const std::wstring& );
      XMLNode importNode( const XMLNode & node, bool deep );
    };
    
    class XMLElement : public XMLNode {
    public:
      virtual ~XMLElement();
      XMLElement();
      XMLElement( const XMLNode & );
      
      XMLElement( xmldomnode_type * );
      
      void setAttribute(const std::wstring& name, const std::wstring& value);
    };
    
    class XMLProcessingInstruction : public XMLNode {
    public:
      virtual ~XMLProcessingInstruction();
      XMLProcessingInstruction();
      XMLProcessingInstruction( const XMLNode& );
      
      XMLProcessingInstruction( xmldomnode_type * );
    };
    
    class XMLAttr : public XMLNode {
    public:
      virtual ~XMLAttr();
      XMLAttr();
      XMLAttr( const XMLNode& );
      XMLAttr( xmldomnode_type * );
    };
    
    class XMLText : public XMLNode {
    public:
      virtual ~XMLText();
      XMLText();
      XMLText( const XMLNode& );
      XMLText( xmldomnode_type * );
    };
    
    class XMLComment : public XMLNode {
    public:
      virtual ~XMLComment();
      XMLComment();
      XMLComment( const XMLComment& );
      XMLComment( xmldomnode_type * );
    };
    
    class XMLNodeList {
    protected:
      
      boost::intrusive_ptr< xmldomnodelist_type > pImpl_;
      
    public:
      virtual ~XMLNodeList();
      XMLNodeList();
      XMLNodeList( const XMLNodeList & other );
      
      XMLNodeList( xmldomnodelist_type * );
      
      size_t size() const;
      XMLNode operator [] (int idx) const;
    };
  }
}

