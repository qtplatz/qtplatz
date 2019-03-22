/**************************************************************************
** Copyright (C) 2010-2019 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2019 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include <adacquire/datawriter.hpp>
#include <vector>

namespace adacquire {
    namespace SignalObserver {
        class DataReadBuffer;
    }
}

namespace socfpga {
    namespace dgmod {

        struct advalue;

        // this was copied to acqrscontrols -- todo replace this as well.
        class data_accessor : public adacquire::SignalObserver::DataAccess {

            data_accessor( const data_accessor& ) = delete;

        public:
            data_accessor();
            data_accessor( std::shared_ptr< const std::vector< socfpga::dgmod::advalue > > data );

            size_t ndata() const override;

            void rewind() override;
            bool next() override;
            uint64_t elapsed_time() const override;
            uint64_t epoch_time() const override;
            uint64_t pos() const override;
            uint32_t fcn() const override;
            uint32_t events() const override;
            size_t xdata( std::string& ) const override;
            size_t xmeta( std::string& ) const override;

            static bool deserialize( std::vector< socfpga::dgmod::advalue >& data, const char* xdata, size_t xsize );
            static void debug_print( const socfpga::dgmod::advalue&, double t0 );

            std::shared_ptr< const std::vector< socfpga::dgmod::advalue > > data_;
            std::vector< advalue >::const_iterator it_;
        };

    }
}
