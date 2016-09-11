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

#include "counting_result.hpp"

namespace adportable {
    namespace counting {

        std::ostream& operator << ( std::ostream& os, const counting_result& t ) {
            
            if ( os.tellp() == std::streamoff(0) )
                os << "## trig#, time-stamp(s), time(s), epoch time(ns), events, scale factor, scale offset, delay time(s),\t"
                    "[time(s), peak-front, peak-tail, apex, apex-intensity, base-intensity]";
            
            if ( auto data = t.data() ) {
                
                os << boost::format( "\n%d, %.8lf, %.8lf, " )
                    % data->serialnumber_ % data->meta_.initialXTimeSeconds % t.data()->timeSinceInject_
                   << t.data()->timeSinceEpoch_
                   << boost::format( ", 0x%08x" ) % t.data()->wellKnownEvents_
                   << boost::format( ", %.8e, %.8e" ) % data->meta_.scaleFactor % data->meta_.scaleOffset
                   << boost::format( ", %.8e" ) % data->meta_.initialXOffset;
                
                if ( ! t.indecies2().empty() ) {
                    for ( auto& idx : t.indecies2() ) {
                        auto v = data->xy( idx.first );
                        os << boost::format( ",\t%.14le, %d, %d, %d, %d, %d" )
                            % v.first
                            % idx.first
                            % idx.second
                            % idx.apex
                            % idx.value
                            % idx.base
                    }
                    
                } else {
                    for ( auto& idx : t.indecies() ) {
                        auto v = data->xy( idx );
                        os << boost::format( ", %.14le, %d" ) % v.first % v.second;
                    }
                }
            }
            return os;
        }

    }
}
