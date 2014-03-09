/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include <cstdint>
#include <string>
#include <adcontrols/controlmethod.hpp>
#include <adportable/float.hpp>
#include "adcontrols_global.h"

namespace adcontrols {

    class ControlMethod;

    namespace controlmethod {

        class MethodItem;

        template<class Native, class Serializer>
        class ADCONTROLSSHARED_EXPORT accessor_t {
        public:
            accessor_t( const std::string& model, uint32_t unitnumber ) : model_( model ), unitnumber_( unitnumber ) {}

            bool setMethod( adcontrols::ControlMethod& dst, const Native& m, uint32_t funcid = 0, double time = (-1) ) const {
                std::string device;
                if ( Serializer::serialize( m, device ) ) {

                    auto it = std::find_if( dst.begin(), dst.end(), [=]( const MethodItem& t ){
                            if ( time < 0 )
                                return t.modelname() == model_ && t.unitnumber() == unitnumber_ && t.uncid() == funcid;
                            else
                                return t.modelname() == model_ && t.unitnumber() == unitnumber_ && t.uncid() == funcid 
                                    && adportable::compare<double>::essentiallyEqual( t.time(), time );
                        });

                    if ( it == dst.end() )
                        it = dst.insert( MethodItem( model_, unitnumber_, funcid ) );
                    it->data( device.data(), device.size() );

                    return true;
                }
                return false; // failed on serialize
            }


        private:
            std::string model_;
            uint32_t unitnumber_;
        };

    }
}
