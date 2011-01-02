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
#include <fstream>
#include <algorithm>

using namespace device_emulator;

TXTSpectrum::TXTSpectrum( const TXTSpectrum& t ) : filename_(t.filename_)
                                                 , tarray_(t.tarray_)
												 , iarray_(t.iarray_)
												 , minValue_(t.minValue_)
												 , maxValue_(t.maxValue_)  
												 , startDelay_(t.startDelay_) 
												 , sampInterval_(t.sampInterval_)
{
} 

bool
TXTSpectrum::load( const std::string& filename )
{
    iarray_.clear();
    tarray_.clear();
    sampInterval_ = 0;

	std::ifstream in( filename.c_str() );
    if ( in.fail() )
		return false;

    double tof, intens;
	do {
      in >> tof;
      in >> intens;

      tarray_.push_back( tof );
      iarray_.push_back( intens );

	} while ( ! in.eof() );

	double t0 = tarray_[0];  // seconds
    double tz = tarray_[ tarray_.size() - 1 ];
    size_t size = tarray_.size();
	double x = (tz - t0) / size;

	sampInterval_ = static_cast<unsigned long>( ( x * 1e12 ) + 0.5 ); // sec -> psec
	startDelay_ = static_cast<unsigned long>( ( t0 * 1.0e12 ) / sampInterval_ + 0.5 );

	minValue_ = *std::min_element( iarray_.begin(), iarray_.end() );
	maxValue_ = *std::max_element( iarray_.begin(), iarray_.end() );

    return true;
}
