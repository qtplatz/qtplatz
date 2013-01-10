/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
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

#ifndef SEQUENCE_HPP
#define SEQUENCE_HPP

#include "adsequence_global.hpp"
#include "schema.hpp"

#include <boost/variant.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/string.hpp>
#include <string>
#include <vector>
#include <map>

namespace boost { namespace serialization { class access; } }
class portable_binary_oarchive;
class portable_binary_iarchive;

namespace adsequence {

    class schema;

    typedef std::vector< char > blob;
    typedef boost::variant< int, double, std::wstring, blob > cdata_t;
    typedef std::vector< cdata_t > line_t;

    class ADSEQUENCESHARED_EXPORT sequence {
    public:
        ~sequence();
        sequence();
        sequence( const sequence& );

        const adsequence::schema& schema() const;
        void schema( const adsequence::schema& );

        size_t size() const;
        void clear();

        void make_line( line_t& ) const;

        line_t& operator [] ( size_t row );
        const line_t& operator [] ( size_t row ) const;
        void operator << ( const line_t& );

    private:
        adsequence::schema * schema_;
        std::vector< std::vector< cdata_t > > lines_;
        std::map< std::wstring, blob > control_methods_;
        std::map< std::wstring, blob > process_methods_;

        friend class boost::serialization::access;
        template<class Archiver> void serialize( Archiver& ar, const unsigned int /* version */ ) {
            ar & schema_
                & lines_
                & control_methods_
                & process_methods_;
        }
    };

}

#endif // SEQUENCE_HPP
