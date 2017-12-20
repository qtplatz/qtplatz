/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "adacquire_global.hpp"
#include "datawriter.hpp"
#include "signalobserver.hpp"

namespace adacquire {

    namespace SignalObserver {
        
        class ADACQUIRESHARED_EXPORT WriteAccess : public DataAccess {

            std::vector< std::shared_ptr< const DataReadBuffer > >::iterator it_;
            WriteAccess( const WriteAccess& ) = delete;
            WriteAccess& operator = ( const WriteAccess& ) = delete;

            std::vector< std::shared_ptr< const DataReadBuffer > > vec_;

        public:
            WriteAccess();
            size_t operator << ( std::shared_ptr< const DataReadBuffer >&& );

            void rewind() override;
            bool next() override;
            size_t ndata() const override;       // number of data in the buffer
            uint64_t elapsed_time() const override;
            uint64_t epoch_time() const override;
            uint64_t pos() const override;       // data address (sequencial number for first data in this frame)
            uint32_t fcn() const override;       // function number for spectrum
            uint32_t events() const override;    // well known events
            size_t xdata( std::string& ) const override;
            size_t xmeta( std::string& ) const override;
        };
        
    }
}

