/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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
#include "contoursmethod.hpp"
#include <algorithm>
#include <limits>

using namespace adcontrols;

ContoursMethod::ContoursMethod() : resize_(1)
                                 , blurSize_(0)
                                 , cannyThreshold_(0)
                                 , szThreshold_{ 0, std::numeric_limits< int >::max() }
{
}

ContoursMethod::ContoursMethod( const ContoursMethod& t ) : resize_( t.resize_ )
                                                          , blurSize_( t.blurSize_ )
                                                          , cannyThreshold_( t.cannyThreshold_ )
                                                          , szThreshold_( t.szThreshold_ )
{
}

ContoursMethod::~ContoursMethod()
{
}

void
ContoursMethod::setSizeFactor( int value )
{
    resize_ = std::max( 1, value );
}

void
ContoursMethod::setBlurSize( int value )
{
    blurSize_ = value;
}

void
ContoursMethod::setCannyThreshold( int value )
{
    cannyThreshold_ = value;
}

void
ContoursMethod::setMinSizeThreshold( unsigned value )
{
    szThreshold_.first = value;
}

void
ContoursMethod::setMaxSizeThreshold( unsigned value )
{
    szThreshold_.second = value;
}

int
ContoursMethod::sizeFactor() const
{
    return std::max( 1, resize_ );
}

int
ContoursMethod::blurSize() const
{
    return blurSize_;
}

int
ContoursMethod::cannyThreshold() const
{
    return cannyThreshold_;
}

unsigned
ContoursMethod::minSizeThreshold() const
{
    return szThreshold_.first;
}

unsigned
ContoursMethod::maxSizeThreshold() const
{
    return szThreshold_.second;
}

