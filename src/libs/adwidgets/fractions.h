// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#ifndef FRACTIONS_H
#define FRACTIONS_H

struct ISADPFractions;

namespace adil {
  namespace ui {

    class Fractions  {
    public:
		~Fractions();
		Fractions( ISADPFractions * pi = 0 );
		Fractions( const Fractions& );

	private:
		ISADPFractions * pi_;
    };

  }
}

#endif // FRACTIONS_H
