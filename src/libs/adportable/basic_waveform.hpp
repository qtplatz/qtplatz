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

    template< typename T, typename M = std::string >
    class basic_waveform {
    public:

        virtual ~basic_waveform() {};

        basic_waveform() : pos_( 0 )
                         , pn_( 0 )
                         , serialnumber_( 0 )
                         , wellKnownEvents_( 0 )
                         , timepoint_( 0 )
                         , elapsed_time_( 0 )
                         , epoch_time_( 0 )
                         , t0_( 0 )
                         , is_ordinal_( true ) {
        }

        basic_waveform(
            uint32_t pos
            , uint32_t pn = 0
            , uint32_t serialnumber = 0
            , uint32_t wellKnownEvents = 0
            , uint64_t timepoint = 0
            , uint64_t elapsed_time = 0
            , uint64_t epoch_time = 0
            , double t0 = 0 ) : pos_( pos )
                              , pn_( pn )
                              , serialnumber_( serialnumber )
                              , wellKnownEvents_( wellKnownEvents )
                              , timepoint_( timepoint )
                              , elapsed_time_( elapsed_time )
                              , epoch_time_( epoch_time )
                              , t0_( t0 )
                              , is_ordinal_( true ) {
        }

        basic_waveform( const basic_waveform& t ) : pos_( t.pos_ )
                                                  , pn_( t.pn_ )
                                                  , serialnumber_( t.serialnumber_ )
                                                  , wellKnownEvents_( t.wellKnownEvents_ )
                                                  , timepoint_( t.timepoint_ )
                                                  , elapsed_time_( t.elapsed_time_ )
                                                  , epoch_time_( t.epoch_time_ )
                                                  , d_( t.d_ )
                                                  , xmeta_( t.xmeta_ )
                                                  , is_ordinal_( t.is_ordinal_ ) {
        }

        typedef T value_type;
        typedef M meta_type;
        typedef typename std::vector< value_type >::iterator iterator_type;
        typedef typename std::vector< value_type >::const_iterator const_iterator_type;

        inline iterator_type begin()             { return d_.begin();      }
        inline iterator_type end()               { return d_.end();        }
        inline const_iterator_type begin() const { return d_.begin();      }
        inline const_iterator_type end() const   { return d_.end();        }
        inline void clear()                      { d_.clear();             }
        inline void resize( size_t d )           { d_.resize( d );         }
        inline iterator_type erase( iterator_type pos ) { return d_.erase( pos ); }
        inline iterator_type erase( const_iterator_type pos ) { return d_.erase( pos ); }
        inline iterator_type erase( iterator_type first, iterator_type last ) { return d_.erase( first, last ); }
        inline iterator_type erase( const_iterator_type first, const_iterator_type last ) { return d_.erase( first, last ); }

        bool is_ordinal() const                  { return is_ordinal_;     }
        void set_is_ordinal( bool d )            { is_ordinal_ = d;        }
        uint64_t timepoint() const               { return timepoint_;      }
        uint64_t elapsed_time() const            { return elapsed_time_;   }
        uint64_t epoch_time() const              { return epoch_time_;     }
        uint32_t pos() const                     { return pos_;            }    // data address (sequencial number)
        uint32_t pn() const                      { return pn_;             }    // protocol number for waveform
        size_t   size() const                    { return d_.size();       }    // number of samples
        uint32_t well_known_events() const       { return wellKnownEvents_;}    // well known events
        uint32_t serialnumber() const            { return serialnumber_;   }    // a.k.a. trigger number
        double t0() const                        { return t0_; }
        template< typename X = uint8_t > const X * xdata() const { return reinterpret_cast< const X* >( d_.data() ); } // binary data access

        void set_timepoint( uint64_t d )         { timepoint_       = d;   }
        void set_elapsed_time( uint64_t d )      { elapsed_time_    = d;   }
        void set_epoch_time( uint64_t d )        { epoch_time_      = d;   }
        void set_pos( uint32_t d )               { pos_             = d;   }    // data address (sequencial number)
        void set_pn( uint32_t d )                { pn_              = d;   }    // protocol number for waveform
        void set_well_known_events( uint32_t d ) { wellKnownEvents_ = d;   }    // well known events
        void set_serialnumber( uint32_t d )      { serialnumber_    = d;   }    // a.k.a. trigger number
        void set_t0( double d )                  { t0_ = d;                }

        void set_xmeta( const M& xmeta )         { xmeta_         = xmeta; }
        const M& xmeta() const                   { return xmeta_;          }    // meta-data

        void emplace_back( T&& t )               { d_.emplace_back( t );   }
        const T& operator []( size_t idx ) const { return d_[ idx ];       }

    protected:
        uint32_t pos_;
        uint32_t pn_;
        uint32_t serialnumber_;
        uint32_t wellKnownEvents_;
        uint64_t timepoint_;
        uint64_t elapsed_time_;
        uint64_t epoch_time_;
        std::vector< T > d_;
        M xmeta_;
        double t0_;
        bool is_ordinal_;
    };
}
