/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
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

#include "signalobserver.hpp"

namespace adicontroler {

    namespace SignalObserver {

        Description::Description() : trace_method_( eTRACE_TRACE )
                                   , spectrometer_( eUnknownSpectrometer )
                                   , axis_x_decimals_( 2 )
                                   , axis_y_decimals_( 0 )
        {
        }

        Description::Description( const Description& t ) : trace_method_( t.trace_method_ )
                                                         , spectrometer_( t.spectrometer_ )
                                                         , trace_id_( t.trace_id_ )
                                                         , trace_display_name_( t.trace_display_name_ )
                                                         , axis_x_label_( t.axis_x_label_ )
                                                         , axis_y_label_( t.axis_y_label_ )
                                                         , axis_x_decimals_( t.axis_x_decimals_ )
                                                         , axis_y_decimals_( t.axis_y_decimals_ )
        {
        }

        eTRACE_METHOD
        Description::trace_method() const
        {
            return trace_method_;
        }

        void
        Description::set_trace_method( eTRACE_METHOD v )
        {
            trace_method_ = v;
        }
        
        eSPECTROMETER
        Description::spectrometer() const
        {
            return spectrometer_;
        }

        void
        Description::set_spectrometer( eSPECTROMETER v )
        {
            spectrometer_ = v;
        }
        
        const char *
        Description::trace_id() const
        {
            return trace_id_.c_str();
        }

        void
        Description::set_trace_id( const std::string& v )
        {
            trace_id_ = v;
        }
        
        const char *
        Description::axis_label( axis id ) const {
            if ( id == axisX )
                return axis_x_label_.c_str();
            else
                return axis_y_label_.c_str();
        }

        void
        Description::set_axis_label( axis id, const std::string& v ) {
            if ( id == axisX )
                axis_x_label_ = v;
            else
                axis_y_label_ = v;
        }
        
        int32_t
        Description::axis_decimals( axis id ) const
        {
            return id == axisX ? axis_x_decimals_ : axis_y_decimals_;
        }

        void
        Description::set_axis_decimals( axis id, int32_t v )
        {
            if ( id == axisX ) axis_x_decimals_ = v; else axis_y_decimals_ = v;
        }

        ///////
        DataReadBuffer::DataReadBuffer() : uptime(0)
                                         , pos(0)
                                         , fcn(0)
                                         , ndata( 0 )
                                         , events( 0 )
        {
        }

        DataReadBuffer::DataReadBuffer( const DataReadBuffer& t ) : uptime( t.uptime )
                                                                  , pos( t.pos )
                                                                  , fcn( t.fcn )
                                                                  , ndata( t.ndata )
                                                                  , events( t.events )
                                                                  , xdata( t.xdata )
                                                                  , xmeta( t.xmeta )
        {
        }

        ///////
        Observer::~Observer()
        {
        }
        
    };

}
