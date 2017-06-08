// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

#include <memory>
#include <utility>
#include <cstdint>
#include <thread>
#include <vector>
#include <functional>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>

namespace dg {

    typedef std::pair< const char *, double > world_value;
    typedef std::pair< const char *, uint32_t > digital_value;
    
    class arpproxy : public std::enable_shared_from_this< arpproxy > {
    public:
        arpproxy();
        ~arpproxy();

        operator bool () const;
        void register_actuals_handler( std::function<void( size_t )> );
        void register_notification_handler( std::function<void( const std::string& )> );        
        void setpts_json_response( std::ostream& o );
        void actuals_json_response( size_t, std::ostream& o );
        void flags_json_response( std::ostream& o );        
        void set( const std::string&, double value );
        void set( const std::string&, bool value );        
            
        class impl;
    private:
        impl * impl_;
    };
}

