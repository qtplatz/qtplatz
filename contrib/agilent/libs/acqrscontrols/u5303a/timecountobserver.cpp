/**************************************************************************
** Copyright (C) 2015-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2015-2016 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@scienceliaison.com
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

#include "timecountobserver.hpp"
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>

using namespace acqrscontrols::u5303a;

static const char * objtext__ = "timecount.1.u5303a.ms-cheminfo.com";
    
TimeCountObserver::TimeCountObserver() : objid_( boost::uuids::name_generator( base_uuid() )( objtext__ ) )
{
    so::Description desc;
    desc.set_trace_method( so::eTRACE_IMAGE_TDC );
    desc.set_spectrometer( so::eMassSpectrometer );
    desc.set_trace_id( objtext__ );  // unique name for the trace, can be used as 'data storage name'
    desc.set_trace_display_name( L"U5303A Threshold Time & Count" );
    desc.set_axis_label( so::Description::axisX, L"Time" );
    desc.set_axis_label( so::Description::axisY, L"Count" );
    desc.set_axis_decimals( so::Description::axisX, 3 );
    desc.set_axis_decimals( so::Description::axisY, 0 );
    setDescription( desc );
}

TimeCountObserver::~TimeCountObserver()
{
}

const char * 
TimeCountObserver::objtext() const
{
    return objtext__;
}

const boost::uuids::uuid&
TimeCountObserver::objid() const
{
    return objid_;
}

uint64_t 
TimeCountObserver::uptime() const 
{
    return 0;
}

void 
TimeCountObserver::uptime_range( uint64_t& oldest, uint64_t& newest ) const 
{
    oldest = newest = 0;
}

std::shared_ptr< so::DataReadBuffer >
TimeCountObserver::readData( uint32_t pos )
{
    return 0;
}

int32_t
TimeCountObserver::posFromTime( uint64_t nsec ) const 
{
    return 0;
}

