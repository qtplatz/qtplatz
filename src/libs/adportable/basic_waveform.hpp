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

        basic_waveform() : pos_( 0 )
                         , fcn_( 0 )
                         , serialnumber_( 0 )
                         , wellKnownEvents_( 0 )
                         , timepoint_( 0 )
                         , elapsed_time_( 0 )
                         , epoch_time_( 0 ) {
        }

        basic_waveform( const basic_waveform& t ) : pos_( t.pos_ )
                                                  , fcn_( t.fcn_ )
                                                  , serialnumber_( t.serialnumber_ )
                                                  , wellKnownEvents_( t.wellKnownEvents_ )
                                                  , timepoint_( t.timepoint_ )
                                                  , elapsed_time_( t.elapsed_time_ )
                                                  , epoch_time_( t.epoch_time_ )
                                                  , d_( t.d_ )
                                                  , xmeta_( t.xmeta_ ) {
        }

        typedef typename std::vector< T >::iterator iterator_type;
        typedef typename std::vector< T >::const_iterator const_iterator_type;

        double time( size_t idx ) const;
        inline iterator_type begin()             { return d_.begin(); }
        inline iterator_type end()               { return d_.end(); }
        inline const_iterator_type begin() const { return d_.begin(); }
        inline const_iterator_type end() const   { return d_.end(); }

        uint64_t timepoint() const       { return timepoint_;       }
        uint64_t elapsed_time() const    { return elapsed_time_;    }
        uint64_t epoch_time() const      { return epoch_time_;      }
        uint32_t pos() const             { return pos_;             }    // data address (sequencial number)
        uint32_t fcn() const             { return fcn_;             }    // function number for waveform
        size_t   size() const            { return d_.size();        }    // number of samples
        uint32_t ndata() const           { return 1;                }    // number of data in the buffer (this is for trace), waveform should always be 1)
        uint32_t wellKnownEvents() const { return wellKnownEvents_; }    // well known events
        uint32_t serialnumber() const    { return serialnumber_;    }    // a.k.a. trigger number

        template< typename X = uint8_t > const X * xdata() const { return reinterpret_cast< const X* >( d_.data() ); }

        void set_xmeta( const std::string& xmeta ) { xmeta_ = xmeta; }
        const std::string& xmeta() const { return xmeta_; }              // serialized meta-data array

        void emplace_back( T&& t )       { d_.emplace_back( t ); };
        const T& operator []( size_t idx ) const { return d_[ idx ]; }

    protected:
        uint32_t pos_;
        uint32_t fcn_;
        uint32_t serialnumber_;
        uint32_t wellKnownEvents_;
        uint64_t timepoint_;
        uint64_t elapsed_time_;
        uint64_t epoch_time_;
        std::vector< T > d_;
        std::string xmeta_;
    };
}
