// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#include "controlmethod.hpp"

using namespace adcontrols;

ControlMethod::~ControlMethod()
{
}

ControlMethod::ControlMethod()
{
}

ControlMethod::ControlMethod( const ControlMethod& t ) : subject_( t.subject_ )
                                                       , description_( t.description_ )
                                                       , lines_( t.lines_ )
{
}

ControlMethod::MethodLine::MethodLine() : unitnumber_( 0 )
                                        , isInitialCondition_( true )
                                        , funcid_( 0 )
{
}

ControlMethod::MethodLine::MethodLine( const MethodLine& t ) : modelname_( t.modelname_ )
                                                             , unitnumber_( t.unitnumber_ )
                                                             , isInitialCondition_( t.isInitialCondition_ )
                                                             , time_( t.time_ )
                                                             , funcid_( t.funcid_ )
                                                             , xdata_( t.xdata_ )
{
}

const std::wstring& 
ControlMethod::MethodLine::modelname() const
{
    return modelname_;
}

void
ControlMethod::MethodLine::modelname( const std::wstring& value )
{
    modelname_ = value;
}

uint32_t
ControlMethod::MethodLine::unitnumber() const
{
    return unitnumber_;
}

void
ControlMethod::MethodLine::unitnumber( uint32_t value ) 
{
    unitnumber_ = value;
}

bool
ControlMethod::MethodLine::isInitialCondition() const
{
    return isInitialCondition_;
}

void
ControlMethod::MethodLine::isInitialCondition( bool value )
{
    isInitialCondition_ = value;
}

const std::pair< uint32_t, uint32_t >&
ControlMethod::MethodLine::time() const
{
    return time_;
}

void
ControlMethod::MethodLine::time( const std::pair< uint32_t, uint32_t >& value )
{
    time_ = value;
}

uint32_t
ControlMethod::MethodLine::funcid() const
{
    return funcid_;
}

void
ControlMethod::MethodLine::funcid( uint32_t value )
{
    funcid_ = value;
}

const uint8_t *
ControlMethod::MethodLine::xdata() const
{
    return xdata_.data();
}

size_t
ControlMethod::MethodLine::xsize() const
{
    return xdata_.size();
}


