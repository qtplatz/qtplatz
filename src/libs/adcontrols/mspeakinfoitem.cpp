// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#include "mspeakinfoitem.hpp"

using namespace adcontrols;

MSPeakInfoItem::~MSPeakInfoItem(void)
{
}

MSPeakInfoItem::MSPeakInfoItem(void) : peak_index_( 0 )
                                     , peak_start_index_( 0 )
                                     , peak_end_index_( 0 )
                                     , base_( 0 )
                                     , mass_(0)
                                     , area_(0)
                                     , height_(0)
                                     , hh_(0)  
{
}

MSPeakInfoItem::MSPeakInfoItem( unsigned int peak_index
                               , double mass
                               , double area
                               , double height
                               , double hh
                               , double time ) : peak_index_( peak_index )
                                               , peak_start_index_( 0 )
                                               , peak_end_index_( 0 )
                                               , base_( 0 )
                                               , mass_(mass)
                                               , area_(area)
                                               , height_(height)
                                               , hh_( hh )
                                               , time_( time ) 
{
}  

MSPeakInfoItem::MSPeakInfoItem( const MSPeakInfoItem& t ) : peak_index_( t.peak_index_ )
                                                          , peak_start_index_( t.peak_start_index_ )
                                                          , peak_end_index_( t.peak_end_index_ )
                                                          , base_( t.base_ )
                                                          , mass_( t.mass_ )
                                                          , area_( t.area_ )
                                                          , height_( t.height_ )
                                                          , hh_( t.hh_ )
                                                          , time_( t.time_ ) 
{
}  

unsigned int
MSPeakInfoItem::peak_index() const
{
    return peak_index_;
}

unsigned int
MSPeakInfoItem::peak_start_index() const
{
    return peak_start_index_;
}

void
MSPeakInfoItem::peak_start_index( unsigned int idx )
{
    peak_start_index_ = idx;
}

unsigned int
MSPeakInfoItem::peak_end_index() const
{
    return peak_end_index_;
}

void
MSPeakInfoItem::peak_end_index( unsigned int idx )
{
    peak_end_index_ = idx;
}

double 
MSPeakInfoItem::base_height() const
{
    return base_;
}

void
MSPeakInfoItem::base_height( double h )
{
    base_ = h;
}

double
MSPeakInfoItem::mass() const
{
    return mass_;
}

double
MSPeakInfoItem::area() const
{
    return area_;
}

double
MSPeakInfoItem::height() const
{
    return height_;
}

double
MSPeakInfoItem::widthHH() const
{
    return hh_;
}

double
MSPeakInfoItem::time() const
{
    return time_;
}