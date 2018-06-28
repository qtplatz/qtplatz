// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
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

#include <adcontrols/timeutil.hpp>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>
#include "adcontrols_global.h"

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT Trace {

        Trace( const Trace& ) = delete;
        Trace& operator = ( const Trace& ) = delete;

    public:
        ~Trace();
		Trace( int fcn = 0, unsigned lower = 2048 - 512, unsigned upper = 2048 );

		unsigned upper_limit;
        unsigned lower_limit;

        [[deprecated("Replaced by setProtocol(int)")]] void set_fcn( size_t fcn );
        [[deprecated("Replaced by protocol()")]] inline int fcn() const { return fcn_; }

        void setProtocol( int );
        inline int protocol() const { return fcn_; }

        void clear();
        size_t size() const;
        void resize( size_t size );

        void setEnable( bool );
        bool enable() const;
        
        void setIsCountingTrace( bool );
        bool isCountingTrace() const;

        double x( size_t ) const;
        double y( size_t ) const;
        std::pair< double, double > xy( size_t ) const;
        uint32_t events( size_t ) const;

        const unsigned long * getEventsArray() const;
        std::pair<double, double> range_y() const;

        bool append( size_t pos, double x, double y, uint32_t events = 0 );
        bool erase_before( size_t pos );
        size_t npos() const;

        void setInjectTime( double );
        double injectTime() const;

        const std::string& legend() const;
        void setLegend( const std::string& );

    private:
		int fcn_;
        double minY_;
        double maxY_;
        bool isCountingTrace_;
        bool enable_;
        double injectTime_;
        std::string legend_;

        enum { data_number, x_value, y_value, event_flags };
        typedef std::tuple< size_t, double, double, uint32_t > value_type;
        std::mutex mutex_;
        std::vector< value_type > values_;
    };

}
