// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
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

#pragma once

#include <string>
#include <map>
#include <iostream>
#include <boost/serialization/version.hpp>
#include <boost/serialization/map.hpp>

namespace adfs {

    class sqlite;
    class blob;
    typedef char char_t;

    namespace internal {

        class attributes {
        public:
            attributes();
            attributes( const attributes& t );

        public:
            operator bool () const;
            
            std::wstring name() const;
            void name( const std::wstring& name );

            std::wstring id() const;
            void id( const std::wstring& );

            std::wstring dataClass() const;
            void dataClass( const std::wstring& );

            std::wstring attribute( const std::wstring& ) const;
            void setAttribute( const std::wstring& key, const std::wstring& value );
            bool fetch();
            bool commit();

            static bool archive( std::ostream&, const attributes& ); // binary
            static bool restore( std::istream&, attributes& ); // binary

            typedef std::map< std::wstring, std::wstring > vector_type;
            vector_type::const_iterator begin() const { return attrib_.begin(); }
            vector_type::const_iterator end() const { return attrib_.end(); }

            virtual boost::int64_t rowid() const = 0;

        protected:
            virtual sqlite& db() const = 0;

        private:
            bool dirty_;
            vector_type attrib_;

            template<class Archive> void serialize( Archive& ar, const unsigned int version ) {
                (void)version;
                ar & BOOST_SERIALIZATION_NVP(attrib_) ;
            }
            friend class boost::serialization::access;
        };

    }
}

BOOST_CLASS_VERSION( adfs::internal::attributes, 1 )
