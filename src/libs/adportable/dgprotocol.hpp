// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC
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

#include <array>

namespace adportable {

    namespace dg {

        class protocol {
        public:
            enum { size = 6 };  // CH0 (push), CH1(INJ), CH2(EXIT), CH3(GATE 0), CH4(GATE 1), CH5(ADC delay)
            
            protocol();
            
            protocol( const protocol& t );

            size_t replicates() const;
            
            void setReplicates( size_t v );
            
            std::pair< double, double >& operator []( int index );
            
            const std::pair< double, double >& operator []( int index ) const;

            const std::array< std::pair< double, double >, size >& pulses() const;
            
        private:
            size_t replicates_;
            std::array< std::pair< double, double >, size > pulses_;
        };
    }
}
