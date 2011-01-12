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
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "msxml.h"

namespace xmlwrapper {
    namespace msxml {

        class XTree {
        public:
            XTree();
            ~XTree();

            XMLDocument& getDocument();
            XMLElement create_dataset( const std::wstring& fullpath );
        private:
            XMLDocument dom_;
            void create_root();
        };
    }
}

#if 0
#include <map>
#include "xmldom.h"

namespace xmlwrapper {

    template<class T> class xtree;
    
    class xtree_node {
    public:
        static XMLNode appendChild(XMLDocument&, XMLElement&, const std::wstring &, int& nLevel );
        static std::wstring newGUID();
        static std::wstring getFullyQualifiedName( const XMLElement& );
    };
    
    template<class T>
    class xtree_node_t {
        XMLElement element_;
        xtree<T> * pTree_;
        int nLevel_;
        
        xtree_node_t( xtree<T> * p ) : pTree_(p), nLevel_(0) {}
        xtree_node_t( xtree<T> * p, const XMLElement & e ) : pTree_(p), element_(e), nLevel_(0) { }
        friend xtree<T>;

    public:
        xtree_node_t() : pTree_(0) {};
        xtree_node_t( const xtree_node_t<T> & t ) : pTree_(t.pTree_), element_(t.element_), nLevel_(t.nLevel_) {}
        xtree_node_t & operator = ( const xtree_node_t & t ) {
            pTree_ = t.pTree_;
            element_ = t.element_;
            nLevel_ = t.nLevel_;
            return *this;
        }
        operator bool () const { return pTree_ && bool(element_);   }
        XMLElement& getElement() { return element_; }
    public:
        xtree_node_t<T> appendChild(const T& t, const std::wstring& name ) {
            XMLElement e = xtree_node::appendChild( pTree_->dom_, element_, name, nLevel_ = 0);
            std::wstring dataId = xtree_node::newGUID();
            e.setAttribute( L"dataId", dataId );
            pTree_->data_[ dataId ] = t;
            return xtree_node_t<T>( pTree_, e );
        }
        
        const std::wstring name() const   { return element_.getNodeName(); }
        const std::wstring dataId() const { return element_.getAttribute( L"dataId" ); }
        
        void setAttribute( const std::wstring& attr, const std::wstring& value ) {
            element_.setAttribute( attr, value );
        }
        
        const std::wstring getAttribute( const std::wstring& attr ) const {
            return element_.getAttribute( attr );
        }
        
        void remove( xtree_node_t<T> & node ) {
            pTree_->remove( node );
        }
        
        operator T& () {
            std::wstring dataId = element_.getAttribute( L"dataId" );
            xtree<T>::vector_type::iterator pos = pTree_->data_.find( dataId );
            if ( pos != pTree_->data_.end() )
                return pos->second;
            throw std::bad_cast();
            
        }
        
        operator const T& () const {
            std::wstring dataId = element_.getAttribute( L"dataId" );
            xtree<T>::vector_type::const_iterator pos = pTree_->data_.find( dataId );
            if ( pos != pTree_->data_.end() )
                return pos->second;
            throw std::bad_cast();
        }
        
        void operator = ( const T& t ) {
            std::wstring dataId = element_.getAttribute( L"dataId" );
            pTree_->data_[dataId] = t;
        }
        
        typedef std::vector<xtree_node_t<T> > nodes_type;
        
        std::vector< xtree_node_t<T> > selectNodes( const std::wstring& query ) {
            std::vector<xtree_node_t<T> > nodes;
            XMLNodeList xmlnodes = element_.selectNodes( query );
            for ( int i = 0; i < int(xmlnodes.size()); ++i )
                nodes.push_back( xtree_node_t<T>(pTree_, xmlnodes[i]) );
            return nodes;
        }
        
        xtree_node_t<T> selectSingleNode( const std::wstring& query ) {
            XMLNode xmlnode = element_.selectSingleNode( query );
            return xtree_node_t<T>(pTree_, xmlnode);
        }
        
        xtree_node_t<T> getParent() {
            XMLNode xnode = element_.selectSingleNode( L".." );
            return xtree_node_t<T>( pTree_, xnode );
        }
        std::wstring getFullyQualifiedName() const {
            return xtree_node::getFullyQualifiedName( element_ );
        }
    };
    
    template <class T>
    class xtree {
    public:
        ~xtree(void) {}
        
        xtree() {
            rootNode_.pTree_ = this;
            XMLHelper::AppendProcessingInstruction(dom_);
            rootNode_.element_ = XMLHelper::AppendClassElement(dom_, XMLElement(), L"xtree", L"xtree");
        }
        xtree( const xtree& t ) : data_(t.data_), rootNode_(this) {
            dom_ = t.dom_;
            rootNode_.element_ = dom_.getDocumentElement();
        }
    private:
        void operator = ( const xtree& ) {}
        
    public:
        typedef xtree_node_t<T> node_type;
        typedef std::vector< node_type > nodeList_type;
        
        typedef T value_type;
        typedef std::map< std::wstring, value_type > vector_type;
        
    private:
        XMLDocument dom_;
        xtree_node_t<T> rootNode_;
        vector_type data_;
        friend xtree_node_t<T>;
        
    public:
        XMLDocument& getDocument() { return dom_; }
        node_type & rootNode() { return rootNode_; }
        inline nodeList_type selectNodes( const std::wstring& q )  { return rootNode_.selectNodes(q); }
        inline node_type selectSingleNode( const std::wstring& q ) { return rootNode_.selectSingleNode(q); }
        void xml( std::wstring& xml ) { dom_.xml( xml ); };
        void loadXML( const std::wstring& xml ) { dom_.loadXML( xml ); }
        void save( const std::wstring& name ) { dom_.save( name ); }
        
        void remove( xtree_node_t<T> & node ) {
            // delete all children/siblings under target node
            xtree<T>::nodeList_type list = node.selectNodes( L".//*" );
            for ( int i = 0; i < int(list.size()); ++i ) {
                vector_type::iterator it = data_.find( list[i].getAttribute( L"dataId" ) );
                if ( it != data_.end() )
                    data_.erase( it );
            }
            vector_type::iterator it = data_.find( node.getAttribute( L"dataId" ) );
            if ( it != data_.end() )
                data_.erase( it );
            
            xtree_node_t<T> parent = node.selectSingleNode(L"..");
            if ( parent )
                parent.element_.removeChild( node.element_ );
            
            node.element_ = XMLElement(0);
        }
        bool get_data( const std::wstring& dataId, T& t ) {
            vector_type::iterator it = data_.find( dataId );
            if ( it != data_.end() ) {
                t = it->second;
                return true;
            }
            return false;
        }
        void set_data( const std::wstring& dataId, const T& t ) {
            data_[dataId] = t;
        }
    };
}
#endif
