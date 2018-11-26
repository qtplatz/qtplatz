// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2019 MS-Cheminformatics LLC
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

#include <vector>
#include <string>

namespace adportable {

    template< typename T >
    class basic_waveform {
    public:
        virtual ~basic_waveform() {};
        typedef typename std::vector< T >::iterator iterator_type;
        typedef typename std::vector< T >::const_iterator const_iterator_type;

        size_t size() const; // number of samples
        double time( size_t idx ) const;
        inline iterator_type begin() { return d_.begin(); }
        inline iterator_type end() { return d_.end(); }
        inline const_iterator_type begin() const { return d_.begin(); }
        inline const_iterator_type end() const { return d_.end(); }

        uint64_t timepoint()    { return timepoint_;    }
        uint64_t elapsed_time() { return elapsed_time_; }
        uint64_t epoch_time()   { return epoch_time_;   }
        uint32_t pos()          { return pos_;          }    // data address (sequencial number for first data in this frame)
        uint32_t fcn()          { return fcn_;          }    // function number for spectrum
        uint32_t ndata()        { return d_.size();     }    // number of data in the buffer (for trace, spectrum should be always 1)
        uint32_t events()       { return events_;       }    // well known events

        template< typename X = uint8_t > const X * xdata() const { return reinterpret_cast< const X* >( d_.data() ); }
        const std::string& xmeta() const { return xmeta_; }    // serialized meta data array

    protected:
        uint32_t serialnumber_;
        uint32_t wellKnownEvents_;
        uint64_t timepoint_;
        uint64_t elapsed_time_;
        uint64_t epoch_time_;
        uint32_t pos_;
        uint32_t fcn_;
        uint32_t events_;
        std::vector< T > d_;
        std::string xmeta_;
    };
}
