/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com
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

#include "constants.hpp"
#include "adicontroller_global.hpp"
//#include <boost/any.hpp>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <compiler/pragma_warning.hpp>

namespace boost { namespace uuids { struct uuid; } }

namespace adicontroller {

    namespace SignalObserver {

        class DataWriter;
        class DataReadBuffer;

#if defined _MSC_VER
        ADICONTROLLERSHARED_TEMPLATE_EXPORT template class ADICONTROLLERSHARED_EXPORT std::weak_ptr < DataWriter > ;
#endif

        class ADICONTROLLERSHARED_EXPORT DataAccess {
            DataAccess( const DataAccess& ) = delete;
            DataAccess& operator = ( const DataAccess& ) = delete;
        public:
            DataAccess();
            virtual void rewind();
            virtual bool next();
            virtual size_t ndata() const;     // number of data in the buffer
            virtual uint64_t elapsed_time() const;
            virtual uint64_t epoch_time() const;
            virtual uint64_t pos() const;       // data address (sequencial number for first data in this frame)
            virtual uint32_t fcn() const;       // function number for spectrum
            virtual uint32_t events() const;    // well known events
            virtual size_t xdata( std::string& ) const;
            virtual size_t xmeta( std::string& ) const;
        };
        
        class ADICONTROLLERSHARED_EXPORT DataWriter : public std::enable_shared_from_this< DataWriter > {

            DataWriter( const DataWriter& ) = delete;
            void operator = ( const DataWriter& ) = delete;
            
        public:
            virtual ~DataWriter();

            DataWriter();
            DataWriter( std::shared_ptr< DataAccess > );

            void rewind();
            bool next();
            
            uint64_t elapsed_time() const;
            uint64_t epoch_time() const;                        
            uint64_t pos() const;       // data address (sequencial number for first data in this frame)
            uint32_t fcn() const;       // function number for spectrum
            uint32_t ndata() const;     // number of data in the buffer
            uint32_t events() const;    // well known events
            size_t xdata( std::string& ) const;
            size_t xmeta( std::string& ) const;
        private:
            std::shared_ptr< DataAccess > accessor_;
        };

    }
}
