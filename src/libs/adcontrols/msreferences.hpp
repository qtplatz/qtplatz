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

#include "adcontrols_global.h"
#include <string>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/version.hpp>

namespace adcontrols {

    class MSReference;

    class ADCONTROLSSHARED_EXPORT MSReferences {
    public:
        MSReferences();
        MSReferences( const MSReferences& );
        typedef MSReference value_type;
        typedef std::vector< value_type > vector_type;

        vector_type::iterator begin();
        vector_type::iterator end();
        vector_type::const_iterator begin() const;
        vector_type::const_iterator end() const;

        const std::wstring& name() const;
        void name( const std::wstring& );
        size_t size() const;
        const MSReference& operator [] ( int idx ) const;
        MSReference& operator [] ( int idx );
        MSReferences& operator << ( const MSReference& );

    private:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            using namespace boost::serialization;
	    (void)version;
                ar & BOOST_SERIALIZATION_NVP(name_);
                ar & BOOST_SERIALIZATION_NVP(vec_);

        }
        vector_type vec_;
        std::wstring name_;
    };
    
}

