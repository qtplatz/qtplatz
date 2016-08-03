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
#include <adportable/threshold_index.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/access.hpp>
#include <memory>
#include <vector>
#include <cstdint>
#include <ostream>
#include <compiler/pragma_warning.hpp>


namespace acqrscontrols {
    namespace u5303a {

        class waveform;

        class ACQRSCONTROLSSHARED_EXPORT threshold_result {
        public:
            threshold_result();
            threshold_result( std::shared_ptr< const waveform > d );
            threshold_result( const threshold_result& t );

            std::shared_ptr< const waveform >& data();

            std::vector< uint32_t >& indecies();
            const std::vector< uint32_t >& indecies() const;

            std::vector< adportable::threshold_index >& indecies2();
            const std::vector< adportable::threshold_index >& indecies2() const;

            std::vector< double >& processed();
            const std::vector< double >& processed() const;

            std::shared_ptr< const waveform > data() const;
            
            const std::pair<uint32_t, uint32_t >& findRange() const;
            uint32_t foundIndex() const;
            void setFoundAction( uint32_t index, const std::pair< uint32_t, uint32_t >& );

# if defined _MSC_VER && _MSC_VER <= 1800
            static const uint32_t npos = (-1);
# else
            static constexpr uint32_t npos = ( -1 );
# endif
            bool deserialize( const int8_t * data, size_t dsize, const int8_t * meta, size_t msize );
            void setFindUp( bool );
            bool findUp() const;
        private:
            std::shared_ptr< const waveform > data_;
            std::vector< uint32_t > indecies_;
            std::vector< adportable::threshold_index > indecies2_;
            std::vector< double > processed_;
            std::pair< uint32_t, uint32_t > findRange_;
            uint32_t foundIndex_;
            bool findUp_;
        };

        // text output
        ACQRSCONTROLSSHARED_EXPORT std::ostream& operator << ( std::ostream&, const threshold_result& );

    }
}
