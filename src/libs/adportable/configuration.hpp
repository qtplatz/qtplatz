// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#include <string>
#include <map>
#include <vector>

namespace adportable {

    class Configuration {
    public:
        ~Configuration(void);
        Configuration(void);
        Configuration( const Configuration& );
        Configuration& operator = ( const Configuration& );

        typedef std::vector<Configuration> vector_type;
        typedef std::map<std::string, std::string> attributes_type;

        const std::string& component() const;
        const std::string& component_interface() const;
        void component_interface( const std::string& );
		
        const std::string& attribute( const std::string& key ) const;
		
        const std::string& name() const;
        void name( const std::string& );
		
        const std::wstring& title() const;
        void title( const std::wstring& );
		
        void attribute( const std::string& key, const std::string& value );
        bool readonly() const;
        bool hasChild() const;
        Configuration& append( const Configuration& );
        void xml( const std::string& );
        inline const std::string& xml() const { return xml_; }
        inline const attributes_type& attributes() const { return attributes_; }
        inline vector_type::iterator begin() { return children_.begin(); }
        inline vector_type::iterator end()   { return children_.end(); }
        inline vector_type::reverse_iterator rbegin() { return children_.rbegin(); }
        inline vector_type::reverse_iterator rend()   { return children_.rend(); }
        inline vector_type::const_iterator begin() const { return children_.begin(); }
        inline vector_type::const_iterator end() const  { return children_.end(); }
		
        static const Configuration * find( const Configuration&, const std::string& );
		
    private:
        std::string xml_;
        std::wstring title_;
        std::string component_interface_;  // <Configuration> <Component interface="value"/> </Configuration>
        attributes_type attributes_;
        std::vector< Configuration > children_;
    };

}
