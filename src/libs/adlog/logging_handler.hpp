/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef LOGGING_HANDLER_HPP
#define LOGGING_HANDLER_HPP

#include <string>
#include <vector>
#include <functional>
#include <mutex>
#include "adlog_global.hpp"
#include <adportable/debug_core.hpp>

#if defined _MSC_VER
# pragma warning(push)
# pragma warning(disable:4251)
#endif

namespace adlog {

    class ADLOGSHARED_EXPORT logging_handler {
        logging_handler();

    public:
        static logging_handler * instance();
        static void log( int pri, const std::string& msg, const std::string& file, int line );

        // actual handler type
        typedef std::function<void( int, const std::string&, const std::string& /* file */, int /* line */)> handler_type;

        void register_handler( handler_type );
		typedef std::vector< handler_type >::iterator iterator;

        void appendLog( int pri, const std::string& msg, const std::string& file, int line );
		void close();

        iterator begin();
        iterator end();
        size_t size() const;

    private:
        static std::mutex mutex_;
        static logging_handler * instance_;
        std::string logfile_;
        std::vector< handler_type > loggers_;
    };

}

#if defined _MSC_VER
# pragma warning(pop)
#endif

#endif // LOGGING_HANDLER_HPP
