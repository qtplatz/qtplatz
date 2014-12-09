/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
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

#include "controlmethodhelper.hpp"
#include "controlmethodC.h"
#include <adportable/debug.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adportable/utf.hpp>

using namespace adinterface;

// static
void
ControlMethodHelper::append( ::ControlMethod::Method& m
                             , const std::string& device
                             , const char * modelname
                             , bool isInitialCondition
                             , double time
                             , uint32_t unitNumber)
{
    m.lines.length( m.lines.length() + 1 ); // add a line
    ::ControlMethod::MethodLine& line = m.lines[ m.lines.length() - 1 ];
    line.modelname = CORBA::string_dup( modelname );
    line.unitnumber = unitNumber;
    line.isInitialCondition = isInitialCondition;
    line.time = time;
    line.xdata.length( static_cast<CORBA::ULong>(device.size()) );
    std::copy( device.begin(), device.end(), line.xdata.get_buffer() );
    
}

// static
void
ControlMethodHelper::replace_or_add( ::ControlMethod::Method& m
                                     , const std::string& device
                                     , const char * modelname
                                     , bool isInitialCondition
                                     , double time
                                     , uint32_t unitNumber)
{
    if ( auto mi = find( m, modelname, unitNumber ) ) {
        mi->modelname = CORBA::string_dup( modelname );
        mi->unitnumber = unitNumber;
        mi->xdata.length( CORBA::ULong( device.size() ) );
        mi->isInitialCondition = isInitialCondition;
        mi->time = time;
        std::copy( device.begin(), device.end(), mi->xdata.get_buffer() );
    } else {
        append( m, device, modelname, isInitialCondition, unitNumber );
    }
}

// static
const ::ControlMethod::MethodLine *
ControlMethodHelper::find( const ::ControlMethod::Method& m
                           , const char * modelname
                           , uint32_t unitNumber )
{
    size_t nlines = m.lines.length();
    for ( size_t i = 0; i < nlines; ++i ) {
        const ControlMethod::MethodLine& line = m.lines[ int( i ) ];
        if ( std::strcmp( modelname, line.modelname.in() ) == 0 && unitNumber == line.unitnumber )
            return &line;
    }
    return 0;
}

// static
::ControlMethod::MethodLine *
ControlMethodHelper::find( ::ControlMethod::Method& m
                           , const char * modelname
                           , uint32_t unitNumber )
{
    size_t nlines = m.lines.length();
    for ( size_t i = 0; i < nlines; ++i ) {
        ControlMethod::MethodLine& line = m.lines[ int(i) ];
        if ( std::strcmp( modelname, line.modelname.in() ) == 0 && unitNumber == line.unitnumber )
            return &line;
    }
    return 0;
}

//static
void
ControlMethodHelper::copy( ::ControlMethod::Method& d, const adcontrols::ControlMethod& s )
{
    d.subject = CORBA::string_dup( s.subject() );
    d.description = CORBA::string_dup( s.description() );

    d.lines.length( CORBA::ULong( s.size() ) );
    CORBA::ULong id = 0;
    for ( auto& item: s ) {
        auto& line = d.lines[ id++ ];
        line.modelname = CORBA::string_dup( item.modelname() );
        line.description = CORBA::string_dup( item.description() );
        line.unitnumber = item.unitnumber();
        line.time = item.time();
        line.isInitialCondition = item.isInitialCondition();
        line.funcid = item.funcid();
        line.itemlabel = CORBA::string_dup( item.itemLabel() );
        line.xdata.length( CORBA::ULong( item.size() ) );
        std::copy( item.data(), item.data() + item.size(), line.xdata.get_buffer() );
    }
}

//static
void
ControlMethodHelper::copy( adcontrols::ControlMethod& d, const ::ControlMethod::Method& s )
{
    d.setSubject( s.subject.in() );
    d.setDescription( s.description.in() );

    size_t nLines = s.lines.length();
    for ( size_t i = 0; i < nLines; ++i ) {
        auto& line = s.lines[ CORBA::ULong( i ) ];
        adcontrols::controlmethod::MethodItem item;
        item.setModelname( line.modelname );
        item.setDescription( line.description );
        item.unitnumber( line.unitnumber );
        item.time( line.time );
        item.isInitialCondition( line.isInitialCondition );
        item.funcid( line.funcid );
        item.setItemLabel( line.itemlabel );
        item.data( reinterpret_cast<const char *>(line.xdata.get_buffer()), line.xdata.length() );
        d.push_back( item );
    }
}

