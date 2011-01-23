//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "mspeakinfoitem.h"

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

MSPeakInfoItem::MSPeakInfoItem( double mass
                               , double area
                               , double height
                               , double hh
                               , double time ) : mass_(mass)
                                               , area_(area)
                                               , height_(height)
                                               , hh_(hh)
                                               , time_( time ) 
{
}  

MSPeakInfoItem::MSPeakInfoItem( const MSPeakInfoItem& t ) : mass_( t.mass_ )
                                                          , area_( t.area_ )
                                                          , height_( t.height_ )
                                                          , hh_( t.hh_ )
                                                          , time_( t.time_ ) 
{
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