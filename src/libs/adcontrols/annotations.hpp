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

#include "adcontrols_global.h"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <vector>
#include "annotation.hpp"

namespace adcontrols {

    class annotation;

    class ADCONTROLSSHARED_EXPORT annotations {
    public:
        annotations();
        annotations( const annotations& );
        typedef annotation value_type;
        typedef std::vector< annotation > vector_type;
        typedef size_t size_type;
        typedef ptrdiff_t difference_type;
        typedef vector_type::iterator iterator;
        typedef vector_type::const_iterator const_iterator;
        typedef annotation& reference;
        typedef const annotation& const_reference;

        enum OrderBy {
            Priority, Index
        };

        size_t size() const;
        bool empty() const;
        void clear();
        void sort( OrderBy order = Priority );
        annotations& operator << ( const annotation& );
        const annotation& operator [] ( size_t ) const;
        annotation& operator [] ( size_t );
        inline iterator begin() { return vec_.begin(); }
        inline iterator end() { return vec_.end(); }
        inline const_iterator begin() const { return vec_.begin(); }
        inline const_iterator end() const { return vec_.end(); }
        inline iterator erase( iterator it ) { return vec_.erase( it ); }
    private:
        vector_type vec_;
        friend class boost::serialization::access;
        template<class Archive>
            void serialize( Archive& ar, const unsigned int /* version */) {
            ar & BOOST_SERIALIZATION_NVP( vec_ );
        }

    };

}

