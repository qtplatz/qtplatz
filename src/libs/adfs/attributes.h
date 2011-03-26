// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

    namespace internal {

        class attributes {
        public:
            attributes();
            attributes( const attributes& );

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

        protected:
            static bool archive( std::ostream&, const attributes& ); // binary
            bool restore( attributes&, std::istream& ); // binary

            virtual inline sqlite& db() const = 0;
            virtual inline boost::int64_t rowid() const = 0;

        private:
            bool dirty_;
            std::map< std::wstring, std::wstring > attrib_;

            template<class Archive> void serialize( Archive& ar, const unsigned int version ) {
                if ( version >= 0 )
                    ar & BOOST_SERIALIZATION_NVP(attrib_) ;
            }
            friend class boost::serialization::access;
        };

    }
}

BOOST_CLASS_VERSION( adfs::internal::attributes, 1 )