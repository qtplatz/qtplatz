// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#ifndef CHROMATOGRAM_H
#define CHROMATOGRAM_H

#include "adcontrols_global.h"
#include <boost/any.hpp>
#include <string>

namespace boost {
  namespace serialization {
	class access;
  }
}

namespace adcontrols {

  namespace internal {
    class ChromatogramImpl;
  }

  class Description;
  class Descriptions;

  class ADCONTROLSSHARED_EXPORT Chromatogram {
  public:
    ~Chromatogram();
    Chromatogram();
    Chromatogram( const Chromatogram& );
	Chromatogram& operator = ( const Chromatogram& );

    size_t size() const;
    void resize( size_t );
    
    const double * getDataArray() const;
    const double * getTimeArray() const;
    const unsigned short * getEventArray() const;

    void addDescription( const Description& );
    const Descriptions& getDescriptions() const;
  private:
	 friend class boost::serialization::access;
	 template<class Archiver> void serialize(Archiver& ar, const unsigned int version);

     internal::ChromatogramImpl * pImpl_;
  };
  
}

#endif // CHROMATOGRAM_H
