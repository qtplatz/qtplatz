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
        typedef std::vector< annotation > vector_type;
        enum OrderBy {
            Priority, Index
        };

        size_t size() const;
        bool empty() const;
        void clear();
        void sort( OrderBy order = Priority );
        operator const vector_type& () const;
        annotations& operator << ( const annotation& );
        const annotation& operator [] ( size_t ) const;
        annotation& operator [] ( size_t );
        inline vector_type::iterator begin() { return vec_.begin(); }
        inline vector_type::iterator end() { return vec_.end(); }
        inline vector_type::const_iterator begin() const { return vec_.begin(); }
        inline vector_type::const_iterator end() const { return vec_.begin(); }
    private:
        vector_type vec_;
        friend class boost::serialization::access;
        template<class Archive>
            void serialize( Archive& ar, const unsigned int /* version */) {
            ar & BOOST_SERIALIZATION_NVP( vec_ );
        }

    };

}

