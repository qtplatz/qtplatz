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
#include "node.h"

namespace boost {
    class any;
}

namespace portfolio {

    namespace internal {
        class PortfolioImpl;
    }

    class Folder;

    class PORTFOLIOSHARED_EXPORT Folium : public internal::Node {
    public:
        ~Folium();
        Folium();
        Folium( const Folium& );
        Folium( xmlNode&, internal::PortfolioImpl * impl );
    public:

        std::wstring path() const;
        bool empty() const;
        void operator = ( const boost::any& );
        operator boost::any& ();
        operator const boost::any& () const;

        std::vector< Folium > attachments();
        const std::vector< Folium > attachments() const;
        Folder getParentFolder();

        typedef std::vector< Folium > vector_type;

        template<class T> static vector_type::iterator find_first_of( vector_type::iterator it, vector_type::iterator ite ) {
            while ( it != ite ) {
                boost::any& data = (*it);
                if ( data.type() == typeid(T) )
                    return it;
                ++it;
            }
            return ite;
        }

        template<class T> static bool get( T& t, Folium& folium ) {
            boost::any& data = folium;
            if ( data.type() == typeid(T) ) {
                t = boost::any_cast<T>(data);
                return true;
            }
            return false;
        }

        // --- create/modify
        Folium addAttachment( const std::wstring& name );
    };

    typedef std::vector< Folium > Folio;

}


