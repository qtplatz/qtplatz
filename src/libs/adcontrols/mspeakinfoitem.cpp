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

MSPeakInfoItem::MSPeakInfoItem( double mass, double area, double height, double hh ) : mass_(mass)
                                                                                     , area_(area)
                                                                                     , height_(height)
                                                                                     , hh_(hh)
{
}  

MSPeakInfoItem::MSPeakInfoItem( const MSPeakInfoItem& t ) : mass_( t.mass_ )
                                                          , area_( t.area_ )
                                                          , height_( t.height_ )
                                                          , hh_( t.hh_ )
{
}  

double
MSPeakInfoItem::mass()
{
    return mass_;
}

double
MSPeakInfoItem::area()
{
    return area_;
}

double
MSPeakInfoItem::height()
{
    return height_;
}

double
MSPeakInfoItem::hh()
{
    return hh_;
}


