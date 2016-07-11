/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "../acqrscontrols_global.hpp"
#include <memory>
#include <vector>
#include <cstdint>
#include <ostream>

namespace adcontrols { class TimeDigitalHistogram; }

namespace acqrscontrols {
    namespace ap240 {

        class waveform;

        class ACQRSCONTROLSSHARED_EXPORT threshold_result {

            std::shared_ptr< const acqrscontrols::ap240::waveform > data_;
            std::vector< uint32_t > indecies_;
            std::vector< double > processed_;
            uint32_t foundIndex_;
            std::pair< uint32_t, uint32_t > findRange_;

        public:
            std::shared_ptr< const acqrscontrols::ap240::waveform >& data();
            std::shared_ptr< const acqrscontrols::ap240::waveform > data() const;

            std::vector< uint32_t >& indecies();
            std::vector< double >& processed();
            const std::vector< uint32_t >& indecies() const;
            const std::vector< double >& processed() const;
            const std::pair<uint32_t, uint32_t >& findRange() const;
            uint32_t foundIndex() const;
            void setFoundAction( uint32_t index, const std::pair< uint32_t, uint32_t >& );

            static constexpr uint32_t npos = (-1);

            bool hasFoundIndex() const { return foundIndex_ != npos; }

            threshold_result();
            threshold_result( std::shared_ptr< const acqrscontrols::ap240::waveform > d );
            threshold_result( const threshold_result& t );
            
            bool deserialize( const int8_t * data, size_t dsize, const int8_t * meta, size_t msize );

            bool operator >> ( adcontrols::TimeDigitalHistogram& x ) const;

        };

        ACQRSCONTROLSSHARED_EXPORT std::ostream& operator << ( std::ostream&, const threshold_result& );

    }
}
