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

#include "../acqrscontrols_global.hpp"
#include "threshold_result.hpp"
#include <adacquire/datawriter.hpp>
#include <vector>

namespace acqrscontrols {
    namespace ap240 {

        class ACQRSCONTROLSSHARED_EXPORT waveform_accessor : public adacquire::SignalObserver::DataAccess {

            std::vector< std::shared_ptr< const acqrscontrols::ap240::waveform > >::iterator it_;
            
        public:
            waveform_accessor();
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
            
            std::vector< std::shared_ptr< const acqrscontrols::ap240::waveform > > list;
        };
        
    }
}

