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

#include "dgprotocols.hpp"
#include <array>
#include <atomic>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>
#include <functional>

// namespace adportable { namespace dg { class protocols; } }

namespace dg {

    class fpga;

    class dgctl {
        std::mutex mutex_;
        dgctl();
    public:
        ~dgctl();
        static dgctl * instance();
        enum { nitem = 6 };
        typedef std::pair<double, double> value_type;
        typedef std::array< value_type, nitem >::iterator iterator;
        typedef std::array< value_type, nitem >::const_iterator const_iterator;

        size_t size() const;
        const value_type& pulse( size_t idx ) const;
        void pulse( size_t idx, const value_type& );

        iterator begin();
        iterator end();
        const_iterator begin() const;
        const_iterator end() const;

        double pulser_interval() const;
        void pulser_interval( double );

        void commit();
        bool activate_trigger();
        bool deactivate_trigger();
        bool is_active() const;
        
        void update();
        bool http_request( const std::string& method, const std::string& request_path, std::string& );
        void register_sse_handler( std::function< void( const std::string&, const std::string&, const std::string& ) > );

    private:
        bool is_active_;
        bool is_dirty_;
        double pulser_interval_;
        std::array< value_type, nitem > pulses_;
        std::vector< std::function< void( const std::string&, const std::string&, const std::string& ) > > event_handlers_;
        // std::shared_ptr< adportable::dg::protocols > protocols_;
    };
}

