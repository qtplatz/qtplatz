/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
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

#include "../adcontrols_global.h"
#include <adcontrols/controlmethod/timedevent.hpp>
#include <boost/variant.hpp>
#include <vector>
#include <string>

namespace adcontrols {

    namespace ControlMethod {

        struct ADCONTROLSSHARED_EXPORT voltage_type {
            double value;
            std::pair< double, double > limits;
            voltage_type( double _1 = 0, std::pair<double, double>&& _2 = { 0,0 } ) : value( _1 ), limits( _2 ) {}
            voltage_type( const voltage_type& t ) : value( t.value ), limits( t.limits ) {}
        };

        struct ADCONTROLSSHARED_EXPORT duration_type {
            double value;
            std::pair< double, double > limits;
            duration_type( double _1 = 0, std::pair<double, double>&& _2 = { 0,0 } ) : value( _1 ), limits( _2 ) {}
            duration_type( const voltage_type& t ) : value( t.value ), limits( t.limits ) {}
        };

        struct ADCONTROLSSHARED_EXPORT switch_type {
            bool value;
            std::pair< std::string, std::string > choice;
            switch_type( bool _1 = false, std::pair<std::string, std::string>&& _2 = { "ON", "OFF" } ) : value( _1 ), choice( _2 ) {}
            switch_type( const switch_type& t ) : value( t.value ), choice( t.choice ) {}
        };

        struct ADCONTROLSSHARED_EXPORT choice_type {
            uint32_t value;
            std::vector< std::string > choice; 
            choice_type( uint32_t _1 = 0 ) : value( _1 ) {}
            choice_type( const choice_type& t ) : value( t.value ), choice( t.choice ) {}
        };

        struct ADCONTROLSSHARED_EXPORT delay_width_type {
            std::pair<double, double> value;
            delay_width_type( std::pair<double, double>&& _1 = { 0, 1.0e-8 } ) : value( _1 ) {}
            delay_width_type( const delay_width_type& t ) : value( t.value ) {}
        };
        
        class ADCONTROLSSHARED_EXPORT EventCap {
        public:
            typedef boost::variant< voltage_type, switch_type, choice_type, delay_width_type > value_type;

            EventCap();
            EventCap( const std::string& item_name, const std::string& item_display_name, const value_type& );
            EventCap( const EventCap& );
            const std::string& item_name() const;
            const std::string& item_display_name() const;
            const value_type& default_value() const;

        private:
            std::string item_name_;             // id
            std::string item_display_name_;
            value_type default_value_;
        };
        
    }
}

