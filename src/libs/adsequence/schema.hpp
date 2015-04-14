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

#include "adsequence_global.hpp"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/string.hpp>
#include <vector>
#include <string>
#if _MSC_VER
# pragma warning(disable:4251)
#endif

namespace adsequence {

    enum SAMPLE_TYPE { SAMPLE_TYPE_UNKNOWN, SAMPLE_TYPE_STD, SAMPLE_TYPE_QC };
	enum COLUMN_TYPE { COLUMN_INT, COLUMN_DOUBLE, COLUMN_VARCHAR, COLUMN_SAMPLE_TYPE };

    class ADSEQUENCESHARED_EXPORT column {
    public:
        column();
        column( const column& );
        column( const std::string& name, const std::string& display_name, COLUMN_TYPE );

        const std::string& name() const;
        const std::string& display_name() const;
        COLUMN_TYPE type() const;
        
    private:
        std::string name_;
        std::string display_name_;
        COLUMN_TYPE type_;

        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int ) {
            ar & BOOST_SERIALIZATION_NVP( name_ )
               & BOOST_SERIALIZATION_NVP( display_name_ )
               & BOOST_SERIALIZATION_NVP( type_ );
        }
    };

    class ADSEQUENCESHARED_EXPORT schema {
    public:
        schema();
        schema( const schema& );

        typedef std::vector< column > vector_type;

		size_t size() const;
		const column& operator [] ( size_t idx ) const;
        vector_type::iterator begin();
        vector_type::iterator end();
        vector_type::const_iterator begin() const;
        vector_type::const_iterator end() const;
        schema& operator << ( const column& );

    private:
        vector_type schema_;

        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int ) {
            ar & BOOST_SERIALIZATION_NVP( schema_ );
        }

    };

}

