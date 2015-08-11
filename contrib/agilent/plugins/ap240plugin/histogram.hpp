/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include <ap240/digitizer.hpp>
#include <adportable/float.hpp>  
#include <atomic>
#include <mutex>

namespace ap240 {

    class threshold_result;

    class histogram {
        histogram( const histogram & ) = delete;
        histogram& operator = ( const histogram& ) = delete;
        std::mutex mutex_;
        std::chrono::steady_clock::time_point tp_;
    public:
        histogram();
        void clear();
        void append( const threshold_result& result );
        size_t trigger_count() const;
        double triggers_per_sec() const;
        size_t getHistogram( std::vector< std::pair<double, uint32_t> >& histogram, ap240::metadata& meta );
    private:
        // metadata for initial trigger in this histogram
        ap240::metadata meta_;  
        std::atomic< size_t > trigger_count_;
        std::vector< uint32_t > data_;
    };


}


