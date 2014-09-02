// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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
#include "serializer.hpp"
#include <cstring>

using namespace adcontrols;

MSPeakInfoItem::~MSPeakInfoItem(void)
{
}

MSPeakInfoItem::MSPeakInfoItem(void)
    : peak_index_(0)
    , peak_start_index_(0)
    , peak_end_index_(0)
    , base_height_(0)
    , mass_(0)
    , area_(0)
    , height_(0)
    , time_from_mass_(0)
    , time_from_time_(0)
    , HH_left_mass_(0)
    , HH_right_mass_(0)
    , HH_left_time_(0)
    , HH_right_time_(0)
    , centroid_left_mass_(0)
    , centroid_right_mass_(0)
    , centroid_left_time_(0)
    , centroid_right_time_(0)
    , centroid_threshold_(0)
    , is_visible_( true )
    , is_reference_( false )
{
}

MSPeakInfoItem::MSPeakInfoItem( const MSPeakInfoItem& t )
    : peak_index_( t.peak_index_)
    , peak_start_index_( t.peak_start_index_)        
    , peak_end_index_( t.peak_end_index_)          
    , base_height_( t.base_height_)             
    , mass_( t.mass_)                    
    , area_( t.area_)                    
    , height_( t.height_)                  
    , time_from_mass_( t.time_from_mass_)          
    , time_from_time_( t.time_from_time_)          
    , HH_left_mass_( t.HH_left_mass_)            
    , HH_right_mass_( t.HH_right_mass_)           
    , HH_left_time_( t.HH_left_time_)            
    , HH_right_time_( t.HH_right_time_)           
    , centroid_left_mass_( t.centroid_left_mass_)      
    , centroid_right_mass_( t.centroid_right_mass_)     
    , centroid_left_time_( t.centroid_left_time_)      
    , centroid_right_time_( t.centroid_right_time_)     
    , centroid_threshold_( t.centroid_threshold_)      
    , is_visible_( t.is_visible_)         
    , is_reference_( t.is_reference_)      
    , formula_( t.formula_ )      
    , annotation_( t.annotation_ )
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
    return base_height_;
}

void
MSPeakInfoItem::base_height( double h )
{
    base_height_ = h;
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
MSPeakInfoItem::widthHH( bool time ) const
{
    return time ? ( HH_right_time_ - HH_left_time_ ) : ( HH_right_mass_ - HH_left_mass_ );
}

double
MSPeakInfoItem::time( bool time ) const
{
    return time ? time_from_time_ : time_from_mass_;
}

double
MSPeakInfoItem::centroid_left( bool time ) const
{
    return time ? centroid_left_time_ : centroid_left_mass_;
}

double
MSPeakInfoItem::centroid_right( bool time ) const
{
    return time ? centroid_right_time_ : centroid_right_mass_;
}

double
MSPeakInfoItem::centroid_threshold() const
{
    return centroid_threshold_;
}

double
MSPeakInfoItem::hh_left_time() const
{
    return HH_left_time_;
}

double
MSPeakInfoItem::hh_right_time() const
{
    return HH_right_time_;
}

const std::string&
MSPeakInfoItem::formula() const
{
    return formula_;
}

void
MSPeakInfoItem::formula( const std::string& formula )
{
    formula_ = formula;
}

const std::wstring&
MSPeakInfoItem::annotation() const
{
    return annotation_;
}

void
MSPeakInfoItem::annotation( const std::wstring& v )
{
    annotation_ = v;
}

void
MSPeakInfoItem::assign_mass( double mass, double left, double right, double HHleft, double HHright )
{
    mass_ = mass;
    centroid_left_mass_ = left;
    centroid_right_mass_ = right;
    HH_left_mass_ = HHleft;
    HH_right_mass_ = HHright;
}

void
MSPeakInfoItem::mass( double mass )
{
    // this will cause when re-assign calibration to centroid spectrum occured
    double d = mass - mass_;

    mass_ = mass;
    centroid_left_mass_ += d;
    centroid_right_mass_ += d;
    HH_left_mass_ += d;
    HH_right_mass_ += d;
}

//static
bool
MSPeakInfoItem::xml_archive( std::wostream& os, const MSPeakInfoItem& t )
{
    return internal::xmlSerializer("MSPeakInfoItem").archive( os, t );
}

//static
bool
MSPeakInfoItem::xml_restore( std::wistream& is, MSPeakInfoItem& t )
{
    return internal::xmlSerializer("MSPeakInfoItem").restore( is, t );
}
