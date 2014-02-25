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

#include "mspeak.hpp"
using namespace adcontrols;

MSPeak::~MSPeak()
{
}

MSPeak::MSPeak() : time_( 0 )               
                 , mass_( 0 )               
                 , mode_( 0 )               
                 , flength_( 0 )         
                 , spectrumIndex_( 0 )
                 , time_width_( 0 )
                 , mass_width_( 0 )
{
}

MSPeak::MSPeak( const MSPeak& t ) : time_( t.time_ )
                                  , mass_( t.mass_ )
                                  , mode_( t.mode_ )
                                  , flength_( t.flength_ )
                                  , formula_( t.formula_ )
                                  , description_( t.description_ )
                                  , spectrumId_( t.spectrumId_ )   
                                  , spectrumIndex_( t.spectrumIndex_ )
                                  , time_width_( t.time_width_ )
                                  , mass_width_( t.mass_width_ )
{
}

MSPeak::MSPeak( double time, double mass, int32_t mode, double flength ) : time_( time )
                                                                         , mass_( mass )
                                                                         , mode_( mode )
                                                                         , flength_( flength )
{
}

MSPeak::MSPeak( const std::string& formula, double mass ) : mass_( mass )
                                                          , mode_( 0 )
                                                          , flength_( 0 )
                                                          , formula_( formula )
                                                          , spectrumIndex_( 0 )
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

double
MSPeak::width( bool isTime ) const
{
    return isTime ? time_width_ : mass_width_;
}

void
MSPeak::width( double value, bool isTime )
{
    if ( isTime )
        time_width_ = value;
    else
        mass_width_ = value;
}

double
MSPeak::flight_length() const
{
    return flength_;
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

const std::string&
MSPeak::spectrumId() const
{
    return spectrumId_;
}

int32_t
MSPeak::spectrumIndex() const
{
    return spectrumIndex_;
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
MSPeak::flight_length( double v )
{
    flength_ = v;
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

void
MSPeak::spectrumIndex( int v )
{
    spectrumIndex_ = v;
}

