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

#ifndef DEBUG_CORE_HPP
#define DEBUG_CORE_HPP

#include <mutex>
#include <memory>

namespace adportable {

    namespace core {
        
        class debug_core : public std::enable_shared_from_this< debug_core > {
            debug_core();
            ~debug_core();
        public:
            static debug_core * instance();

            bool open( const std::string& logfile );
            const std::string& logfile() const;

            virtual void log( int pri, const std::string& msg, const std::string& file, int line ) const;
            void hook( debug_core * );

        private:
            std::string logfname_;
            std::shared_ptr< debug_core > hook_;
            static std::mutex mutex_;
            static debug_core * instance_;
        };
        
    }
}

#endif // DEBUG_CORE_HPP
