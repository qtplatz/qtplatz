// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2019 MS-Cheminformatics LLC
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

#include "threshold_index.hpp"
#include "counting_result.hpp"
#include <vector>
#include <string>

namespace adportable {
    namespace counting {

        template< typename value_type, typename meta_type, typename index_type = threshold_index >
        class basic_histogram : public adportable::basic_waveform< index_type, meta_type > {
        public:
            virtual ~basic_histogram() {};

            basic_histogram() : algo_( Absolute ), threshold_level_( 0 ) {}

            inline void set_algo( algo d )               { algo_ = d; }
            inline const enum algo& algo() const         { return algo_; }

            inline void set_threshold_level( double d )  { threshold_level_ = d; }
            inline const double& threshold_level() const { return threshold_level_; }

            // inline const std::vector< index_type >& indices2() const { return (*this); };

        protected:
            enum algo algo_;
            double threshold_level_;
        };

    }
}
