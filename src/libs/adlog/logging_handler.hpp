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

#ifndef LOGGING_HANDLER_HPP
#define LOGGING_HANDLER_HPP

#include <atomic>
#include <chrono>
#include <functional>
#include <mutex>
#include <string>
#include <vector>

#include "adlog_global.hpp"
#include <adportable/debug_core.hpp>
#include <boost/signals2.hpp>

#if defined _MSC_VER
# pragma warning(push)
# pragma warning(disable:4251)
#endif

namespace adlog {

    class ADLOGSHARED_EXPORT logging_handler {
        logging_handler();

    public:
        static logging_handler * instance();
        // static void log( int pri, const std::string& msg, const std::string& file, int line );

        // actual handler type
        typedef boost::signals2::signal<void(int /* pri */
                                             , const std::string& /* text */
                                             , const std::string& /* file */
                                             , int /* line */
                                             , const std::chrono::system_clock::time_point& ) > handler_type;

        boost::signals2::connection register_handler( handler_type::slot_type );
		typedef std::vector< handler_type >::iterator iterator;

        void appendLog( int pri, const std::string& msg, const std::string& file, int line, const std::chrono::system_clock::time_point& );
		void close();

        void setpid( uint64_t id );
        void setlogfile( const std::string& );
        const std::string& logfile() const;

    private:
        static std::mutex mutex_;
        std::string logfile_;
        handler_type logger_;
    };

}

#if defined _MSC_VER
# pragma warning(pop)
#endif

#endif // LOGGING_HANDLER_HPP
