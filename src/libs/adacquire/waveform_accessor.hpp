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

#include "adacquire_global.hpp"
#include "datawriter.hpp"
#include <adcontrols/timedigitalhistogram.hpp>
#include <vector>

namespace adacquire {

    template< typename waveform_type >
    class waveform_accessor_ : public adacquire::SignalObserver::DataAccess {

        typename std::vector< std::shared_ptr< const waveform_type > >::iterator it_;

        waveform_accessor_( const waveform_accessor_& ) = delete;

    public:

        waveform_accessor_()                           { }

        size_t ndata() const override                  { return d_.size();                     }
        void rewind() override                         { it_ = d_.begin();                     }
        bool next() override                           { return ++it_ != d_.end();             }
        uint64_t elapsed_time() const override         { return (*it_)->elapsed_time();        }
        uint64_t epoch_time() const override           { return (*it_)->epoch_time();          }
        uint64_t pos() const override                  { return (*it_)->serialnumber();        }
        uint32_t fcn() const override                  { return ( *it_ )->pn();                }
        uint32_t events() const override               { return (*it_)->well_known_events();   }
        size_t xdata( std::string& ar ) const override { return (*it_)->serialize_xdata( ar ); }
        size_t xmeta( std::string& ar ) const override { return (*it_)->serialize_xmeta( ar ); }

        std::vector< std::shared_ptr< const waveform_type > > d_;
    };

}
