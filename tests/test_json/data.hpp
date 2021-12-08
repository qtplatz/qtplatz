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
        };
    }
    struct adc {
        uint64_t tp;
        uint32_t nacc;
        std::vector< double > values;
        adc() : tp(0), nacc(0), values(0) {}
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
};
