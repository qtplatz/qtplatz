/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
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

#include "mspeak.hpp"
using namespace adcontrols;

MSPeak::~MSPeak()
{
}

MSPeak::MSPeak()
{
}

MSPeak::MSPeak( const MSPeak& t ) : time_( t.time_ )
                                  , mass_( t.mass_ )
                                  , mode_( t.mode_ )
                                  , formula_( t.formula_ )
                                  , description_( t.description_ )
                                  , spectrumId_( t.spectrumId_ )
{
}

double
MSPeak::time() const
{
    return time_;
}

double
MSPeak::mass() const
{
    return mass_;
}

int32_t
MSPeak::mode() const
{
    return mode_;
}

const std::string&
MSPeak::formula() const
{
    return formula_;
}

const std::wstring&
MSPeak::description() const
{
    return description_;
}

void
MSPeak::time( double v )
{
    time_ = v;
}

void
MSPeak::mass( double v )
{
    mass_ = v;
}

void
MSPeak::mode( int32_t v )
{
    mode_ = v;
}

void
MSPeak::formula( const std::string& v )
{
    formula_= v;
}

void
MSPeak::description( const std::wstring& v )
{
    description_ = v;
}

void
MSPeak::spectrumId( const std::string& v )
{
    spectrumId_ = v;
}

