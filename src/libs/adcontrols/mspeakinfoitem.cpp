// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

MSPeakInfoItem::MSPeakInfoItem(void) : mass_(0)
                                     , area_(0)
                                     , height_(0)
                                     , hh_(0)  
{
}

MSPeakInfoItem::MSPeakInfoItem( unsigned int index
                               , double mass
                               , double area
                               , double height
                               , double hh
                               , double time ) : index_( index )
                                               , mass_(mass)
                                               , area_(area)
                                               , height_(height)
                                               , hh_(hh)
                                               , time_( time ) 
{
}  

MSPeakInfoItem::MSPeakInfoItem( const MSPeakInfoItem& t ) : index_( t.index_ )
                                                          , mass_( t.mass_ )
                                                          , area_( t.area_ )
                                                          , height_( t.height_ )
                                                          , hh_( t.hh_ )
                                                          , time_( t.time_ ) 
{
}  

unsigned int
MSPeakInfoItem::index() const
{
    return index_;
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