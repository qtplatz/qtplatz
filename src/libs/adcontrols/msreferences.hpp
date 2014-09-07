// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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
#include <vector>
#include <memory>
#include <boost/serialization/version.hpp>

namespace boost { namespace serialization { class access; } }

namespace adcontrols {

    class MSReference;

    class ADCONTROLSSHARED_EXPORT MSReferences {
    public:
        ~MSReferences();
        MSReferences();
        MSReferences( const MSReferences& );
        MSReferences& operator = ( const MSReferences& );

        typedef MSReference value_type;
        typedef std::vector< value_type > vector_type;
        typedef vector_type::iterator iterator;
        typedef vector_type::const_iterator const_iterator;

        iterator begin();
        iterator end();
        const_iterator begin() const;
        const_iterator end() const;
        void clear();

        const wchar_t * name() const;
        void name( const wchar_t * );
        size_t size() const;
        const MSReference& operator [] ( int idx ) const;
        MSReference& operator [] ( int idx );
        MSReferences& operator << ( const MSReference& );

    private:
#   if  defined _MSC_VER
#   pragma warning(disable:4251)
#   endif
        class impl;
        std::unique_ptr< impl > impl_;

        friend class boost::serialization::access;
        template<class Archive>
        void serialize( Archive& ar, const unsigned int version );
    };
    
}

