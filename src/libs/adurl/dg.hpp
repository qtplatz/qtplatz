// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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

#include "adurl_global.h"
#include <adportable/dgprotocols.hpp>
#include <array>
#include <memory>
#include <string>

//namespace adportable { namespace dg { class protocols; } }

namespace adurl {

    class ADURLSHARED_EXPORT dg {
    public:

        enum { size = 6 };  // CH0 (push), CH1(INJ), CH2(EXIT), CH3(GATE 0), CH4(GATE 1), CH5(ADC delay)
        static constexpr const size_t number_of_channels = adportable::dg::delay_pulse_count;
        static constexpr const size_t number_of_protocols = 4;

        class ADURLSHARED_EXPORT request_timeout : public  std::exception {};
        class ADURLSHARED_EXPORT error_reply : public std::exception {};    

        /** \brief constructor, which take server address
         */
        dg( const char * server );
        
        /** \brief commit (set) delay pulse to delay generator-box
         */
        bool commit( const adportable::dg::protocols<adportable::dg::protocol<> >& );

        /** \brief update (read) actual delay pulse data from delay-generator box
         *  
         * updated results can be read using interval() and delayPuse() methods
         */        
        [[deprecated]] bool update_actuals();

        /** \brief start (activate) triggers
         */
        bool start_triggers();

        /** \brief stop (deactivate) triggers
         */        
        bool stop_triggers();

        /** \brief reset error state
        */
        void resetError();

        bool fetch( adportable::dg::protocols< adportable::dg::protocol<> >& );
        bool fetch( std::string& json );

    private:
        std::string server_;
        bool dirty_;
        bool errorState_;
        const int timeout_ = 3000;
    };
    
}
