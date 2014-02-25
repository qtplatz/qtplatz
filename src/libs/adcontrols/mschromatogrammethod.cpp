/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "mschromatogrammethod.hpp"

using namespace adcontrols;

MSChromatogramMethod::MSChromatogramMethod() : dataSource_( Profile )
                                             , widthMethod_( widthInDa )
                                             , mass_limits_( -1, -1 )
{
	width_[ widthInDa ] = 0.001;
	width_[ widthInRP ] = 100000;
}

MSChromatogramMethod::MSChromatogramMethod( const MSChromatogramMethod& t ) : dataSource_( t.dataSource_ )
                                                                            , widthMethod_( t.widthMethod_ )
                                                                            , width_( t.width_ )
                                                                            , mass_limits_( t.mass_limits_ )
{
}

MSChromatogramMethod::DataSource
MSChromatogramMethod::dataSource() const
{
	return dataSource_;
}

void
MSChromatogramMethod::dataSource( MSChromatogramMethod::DataSource v )
{
	dataSource_ = v;
}


MSChromatogramMethod::WidthMethod
MSChromatogramMethod::widthMethod() const
{
    return widthMethod_;
}

void
MSChromatogramMethod::widthMethod( MSChromatogramMethod::WidthMethod method )
{
    widthMethod_ = method;
}

double
MSChromatogramMethod::width( WidthMethod method ) const
{
    //assert( size_t(method) < width_.size() );
    return width_[ method ];
}

void
MSChromatogramMethod::width( double value, WidthMethod method )
{
    //assert( size_t(method) < width_.size() );
    width_[ method ] = value;
}

double
MSChromatogramMethod::lower_limit() const
{
    return mass_limits_.first;
}

double
MSChromatogramMethod::upper_limit() const
{
    return mass_limits_.second;

}

void
MSChromatogramMethod::lower_limit( double v )
{
    mass_limits_.first = v;
}

void
MSChromatogramMethod::upper_limit( double v )
{
    mass_limits_.second = v;
}

