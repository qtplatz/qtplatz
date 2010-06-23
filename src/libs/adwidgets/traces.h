// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef TRACES_H
#define TRACES_H

struct ISADPTraces;

namespace adil {
  namespace ui {

	  class Trace;

	  class Traces {
	  public:
		  ~Traces();
		  Traces( ISADPTraces * pi = 0 );
		  Traces( const Traces& );
	  public:
		  Trace item(long Index);
		  size_t size() const;
		  Trace add();
		  void remove(long Index);
		  void clear();
		  bool visible() const;
		  void visible(bool newValue);
      
	    Trace operator[](long idx);
	    

	  private:
		  ISADPTraces * pi_;
	  };
  }
}

#endif // TRACES_H
