/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC / Advanced Instrumentation Project
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

#ifndef SEQUENCE_HPP
#define SEQUENCE_HPP

#include "adsequence_global.hpp"
#include "schema.hpp"
#include <boost/smart_ptr.hpp>
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
namespace adcontrols { class ProcessMethod; }

namespace adsequence {

    class schema;

    typedef std::vector< char > blob;
    typedef boost::variant< int, double, std::wstring > cdata_t;
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

        line_t make_line() const;

        line_t& operator [] ( size_t row );
        const line_t& operator [] ( size_t row ) const;

        void operator << ( const line_t& );

        typedef std::map< std::wstring, blob> method_vector_type;

		method_vector_type& getControlMethod();
        const method_vector_type& getControlMethod() const;

		method_vector_type& getProcessMethod();
        const method_vector_type& getProcessMethod() const;

		static bool xml_archive( std::ostream&, const sequence& );
		static bool xml_restore( std::istream&, sequence& );
		static bool archive( std::ostream&, const sequence& );
		static bool restore( std::istream&, sequence& );

    private:
        adsequence::schema * schema_;
        std::vector< std::vector< cdata_t > > lines_;

        std::map< std::wstring, blob > control_methods_;
        std::map< std::wstring, blob > process_methods_;

        friend class boost::serialization::access;
        template<class Archiver> void serialize( Archiver& ar, const unsigned int /*version*/ ) {
            ar & BOOST_SERIALIZATION_NVP( schema_ );
			ar & BOOST_SERIALIZATION_NVP( lines_ );
			ar & BOOST_SERIALIZATION_NVP( control_methods_ );
			ar & BOOST_SERIALIZATION_NVP( process_methods_ );
        }
    };

}

#endif // SEQUENCE_HPP
