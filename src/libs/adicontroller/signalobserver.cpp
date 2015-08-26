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

        Description::Description() : trace_method( eTRACE_TRACE )
                                   , spectrometer( eUnknownSpectrometer )
                                   , axis_x_decimals( 2 )
                                   , axis_y_decimals( 0 )
        {
        }

        Description::Description( const Description& t ) : trace_method( t.trace_method )
                                                         , spectrometer( t.spectrometer )
                                                         , trace_id(t.trace_id)
                                                         , trace_display_name(t.trace_display_name)
                                                         , axis_x_label(t.axis_x_label)
                                                         , axis_y_label(t.axis_y_label )
                                                         , axis_x_decimals(t.axis_x_decimals)
                                                         , axis_y_decimals(t.axis_y_decimals)
        {
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
