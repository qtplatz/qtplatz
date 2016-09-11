/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#include "threshold_index.hpp"
#include <vector>

namespace adportable {
    namespace counting {

        class counting_result {
        public:
            static constexpr uint32_t npos = ( -1 );
            enum algo { Absolute, AverageRelative, Deferential };
            
            counting_result() : algo_( Absolute )
                              , threshold_level_( 0 ) {
            }
            
            counting_result( const counting_result& t ) : indecies2_( t.indecies2_ )
                                                        , threshold_level_( t.threshold_level_ ){
            }

            inline std::vector< adportable::counting::threshold_index >& indecies2() {
                return indecies2_;
            }

            inline const std::vector< adportable::counting::threshold_index >& indecies2() const {
                return indecies2_;
            }

            inline enum algo& algo() {
                return algo_;
            }
            
            inline const enum algo& algo() const {
                return algo_;
            }
            
            inline double& threshold_level() {
                return threshold_level_;
            }

            inline const double& threshold_level() const {
                return threshold_level_;
            }

        protected:
            std::vector< adportable::counting::threshold_index > indecies2_;
            enum algo algo_;
            double threshold_level_;
        };
    }
}
