/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "adcontrols_global.h"
#include <compiler/disable_dll_interface.h>
#include <boost/serialization/version.hpp>
#include <memory>
#include <string>
#include <vector>

namespace boost { namespace serialization { class access; } }

namespace adcontrols {

    class moltable;

    class ADCONTROLSSHARED_EXPORT MSSimulatorMethod {
    public:
        ~MSSimulatorMethod();
        MSSimulatorMethod();
        MSSimulatorMethod( const MSSimulatorMethod& );
        MSSimulatorMethod& operator = ( const MSSimulatorMethod& );
        
        static const wchar_t * dataClass() { return L"adcontrols::MSSimulatorMethod"; }

        double lower_limit() const;
        double upper_limit() const;
        
        void set_lower_limit( double );
        void set_upper_limit( double );

        uint32_t charge_state_min() const;
        uint32_t charge_state_max() const;
        
        void set_charge_state_min( uint32_t );
        void set_charge_state_max( uint32_t );        

        void set_resolving_power( double );
        double resolving_power() const;

        bool is_positive_polarity() const;
        void set_is_positive_polarity( bool );

        bool is_tof() const;
        void set_is_tof( bool );
        double length() const;
        void set_length( double );
        double accelerator_voltage() const;
        void set_accelerator_voltage( double );        
        double tDelay() const;
        void set_tDelay( double );        

        const moltable& molecules() const;
        moltable& molecules();
        void set_molecules( const moltable& );
        
    private:
        class impl;
        std::unique_ptr< impl > impl_;

        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int version );
    };
    
}


