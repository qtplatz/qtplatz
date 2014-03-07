// This is a -*- C++ -*- header.
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
                                                       , items_( t.items_ )
{
}

// ControlMethod&
// ControlMethod::operator = ( const ControlMethod& t )
// {
//     subject_ = t.subject_;
//     description_ = t.description_;
//     items_ = t.items_;
//     return *this;
// }

ControlMethod::iterator
ControlMethod::begin()
{
    return items_.begin();
}

ControlMethod::iterator
ControlMethod::end()
{
    return items_.end();
}

ControlMethod::const_iterator
ControlMethod::begin() const
{
    return items_.begin();
}

ControlMethod::const_iterator
ControlMethod::end() const
{
    return items_.end();
}

ControlMethod::iterator
ControlMethod::erase( iterator pos )
{
    return items_.erase( pos );
}

ControlMethod::iterator
ControlMethod::erase( iterator first, iterator last )
{
    return items_.erase( first, last );
}

size_t
ControlMethod::size() const 
{
    return items_.size();
}

using namespace adcontrols::controlmethod;

MethodItem::MethodItem() : unitnumber_( 0 )
                         , isInitialCondition_( true )
                         , time_( -1 )
                         , funcid_( 0 )
{
}

MethodItem::MethodItem( const MethodItem& t ) : modelname_( t.modelname_ )
                                              , unitnumber_( t.unitnumber_ )
                                              , isInitialCondition_( t.isInitialCondition_ )
                                              , time_( t.time_ )
                                              , funcid_( t.funcid_ )
                                              , label_( t.label_ )
                                              , data_( t.data_ )
{
}

const std::string& 
MethodItem::modelname() const
{
    return modelname_;
}

void
MethodItem::modelname( const std::string& value )
{
    modelname_ = value;
}

uint32_t
MethodItem::unitnumber() const
{
    return unitnumber_;
}

void
MethodItem::unitnumber( uint32_t value ) 
{
    unitnumber_ = value;
}

bool
MethodItem::isInitialCondition() const
{
    return isInitialCondition_;
}

void
MethodItem::isInitialCondition( bool value )
{
    isInitialCondition_ = value;
    time_ = (-1);
}

const double&
MethodItem::time() const
{
    return time_;
}

void
MethodItem::time( const double& value )
{
    isInitialCondition_ = false;
    time_ = value;
}

uint32_t
MethodItem::funcid() const
{
    return funcid_;
}

void
MethodItem::funcid( uint32_t value )
{
    funcid_ = value;
}

void
MethodItem::itemLabel( const std::string& value )
{
    label_ = value;
}

const std::string&
MethodItem::itemLabel() const
{
    return label_;
}

const char *
MethodItem::data() const
{
    return data_.data();
}

size_t
MethodItem::size() const
{
    return data_.size();
}


