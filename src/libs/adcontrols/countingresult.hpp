// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
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
#include <string>
#include <tuple>
#include <vector>

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT CountingResult {
    public:
        CountingResult();
        CountingResult( const CountingResult& );
        CountingResult& operator = ( const CountingResult& );

        typedef std::tuple< bool
                            , std::string                   // formula
                            , std::pair< size_t, size_t > // (count, ntrig)
                            , int
                            > value_type;
        enum {
            CountingEnable
            , CountingFormula
            , CountingRate
            , CountingProtocol
        };

        typedef std::vector< value_type >::iterator iterator;
        typedef std::vector< value_type >::const_iterator const_iterator;

        size_t size() const;
        void resize( size_t );
        void clear();
        value_type& operator [] ( size_t );
        const value_type& operator [] ( size_t ) const;
        iterator begin();
        iterator end();
        const_iterator begin() const;
        const_iterator end() const;

        CountingResult& operator << ( value_type&& );

    private:
        std::vector< value_type > values_;
    };

}



