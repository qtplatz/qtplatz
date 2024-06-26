// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2018 MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: toshi.hondo@qtplatz.com
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

#include <cstdint>
#include <string>
#include <vector>

bool is_equal( double a, double b );
bool is_equal( const std::vector< double>&, const std::vector< double >& );

namespace tick {
    namespace hv {
        struct value {
            uint32_t id;
            std::string name;
            uint32_t sn;
            double set;
            double act;
            std::string unit;
            value() : id(0), name(""), sn(0), set(0), act(0), unit("") {}
            value( uint32_t _id, const std::string _name, uint32_t _sn, double _set, double _act, const std::string& _unit )
                : id(_id), name(_name), sn(_sn), set(_set), act(_act), unit(_unit) {}
            value( const value& t ) : id( t.id )
                                    , name( t.name )
                                    , sn( t.sn )
                                    , set( t.set )
                                    , act( t.act )
                                    , unit( t.unit ) {
            }

            bool operator == ( const value& t ) const {
                return
                    id == t.id &&
                    name == t.name &&
                    sn == t.sn &&
                    is_equal( set, t.set );
            }
        };
    }
    struct adc {
        uint64_t tp;
        uint32_t nacc;
        std::vector< double > values;
        adc() : tp(0), nacc(0), values(0) {}
        adc( const adc& t ) : tp( t.tp )
                            , nacc( t.nacc )
                            , values( t.values ) {
        }
        bool operator == ( const adc& t ) const {
            return tp == t.tp &&
                nacc == t.nacc &&
                is_equal( values, t.values);
        }
        std::string compare( const adc& t, const std::string& p ) const {
            return !(tp == t.tp) ? ( p + ".tp" ) :
                !(nacc == t.nacc) ? ( p + ".nacc" ) :
                !is_equal( values, t.values ) ? ( p + ".values" ) : "";
        }
    };
}

struct data {
public:
    uint32_t tick;
    uint64_t time;
    uint32_t nsec;
    std::vector< tick::hv::value > values;
    std::string alarm;
    tick::adc adc;
    data() : tick(0), time(0), nsec(0),values(0), alarm("") {}
    data( const data& t ) : tick( t.tick )
                          , time( t.time )
                          , nsec( t.nsec )
                          , values( t.values )
                          , alarm( t.alarm )
                          , adc( t.adc ) {
    }
    bool operator == ( const data& t ) const {
        return
            tick == t.tick &&
            time == t.time &&
            nsec == t.nsec &&
            values == t.values &&
            alarm == t.alarm &&
            adc == t.adc;
        return false;
    }
    std::string compare( const data& t ) const {
        return !(tick == t.tick) ? "tick" :
            !( time == t.time ) ? "time" :
            !( nsec == t.nsec ) ? "nsec" :
            !( values == t.values ) ? "values" :
            !( alarm == t.alarm ) ? "alarm" :
            !(adc == t.adc) ? adc.compare( t.adc, "adc" ) : "";
    }
};
