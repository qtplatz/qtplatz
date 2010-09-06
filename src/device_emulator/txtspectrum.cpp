// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

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
