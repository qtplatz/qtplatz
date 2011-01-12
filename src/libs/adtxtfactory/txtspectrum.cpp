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

#include "txtspectrum.h"
#include <adportable/string.h>
#include <fstream>
#include <algorithm>

using namespace adtxtfactory;

TXTSpectrum::TXTSpectrum()
{
}

bool
TXTSpectrum::load( const std::wstring& name )
{
    std::string filename = adportable::string::convert( name );

    std::ifstream in( filename.c_str() );
    if ( in.fail() )
        return false;
   
    double time, mass, intens;
    do {
        in >> time;
        in >> mass;
        in >> intens;
        timeArray_.push_back( time );
        massArray_.push_back( mass );
        intensArray_.push_back( intens );
    } while( in.eof() );

    double t0 = timeArray_[0];  // seconds
    double tz = timeArray_[ timeArray_.size() - 1 ];
    size_t size = timeArray_.size();
	double x = (tz - t0) / size;

    sampInterval_ = static_cast<unsigned long>( ( x * 1e12 ) + 0.5 ); // sec -> psec
	startDelay_ = static_cast<unsigned long>( ( t0 * 1.0e12 ) / sampInterval_ + 0.5 );

	minValue_ = *std::min_element( intensArray_.begin(), intensArray_.end() );
	maxValue_ = *std::max_element( intensArray_.begin(), intensArray_.end() );

    return true;
}