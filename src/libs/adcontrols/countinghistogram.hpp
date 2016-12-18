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

    class CountingPeak;
    class CountingData;
#if defined _MSC_VER
    template class ADCONTROLSSHARED_EXPORT std::vector< adcontrols::CountingPeak >;
#endif

    class ADCONTROLSSHARED_EXPORT CountingHistogram {
    public:

        CountingHistogram();
        CountingHistogram( const CountingHistogram& );
        CountingHistogram& operator = ( const CountingHistogram& );

        CountingHistogram& operator << ( const CountingData& );        

        void setXIncrement( double );
        double xIncrement() const;

        typedef std::vector< std::pair< double, std::vector< adcontrols::CountingPeak > > >::const_iterator const_iterator;
        typedef std::vector< std::pair< double, std::vector< adcontrols::CountingPeak > > >::iterator iterator;

        const_iterator begin() const;
        const_iterator end() const;
        iterator begin();
        iterator end();
        void clear();
        size_t size() const;

    private:
        double xIncrement_; // sampling interval (seconds)
        std::vector< std::pair< double, std::vector< adcontrols::CountingPeak > > > pklists_;
    };

}



