// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef TABLEOFELEMENTS_H
#define TABLEOFELEMENTS_H

#include "adcontrols_global.h"
#include <string>

namespace adcontrols {

  namespace internal {
    class TableOfElementsImpl;
  }

  class Element;
  class Elements;
  class SuperAtom;
  class SuperAtoms;

  class ADCONTROLSSHARED_EXPORT TableOfElements {
    ~TableOfElements();
    TableOfElements();
  public:
    static TableOfElements * instance();
    void dispose();

    std::wstring saveXml() const;
    void loadXml( const std::wstring& );

  private:
    static TableOfElements * instance_;
    internal::TableOfElementsImpl * pImpl_;
  };

}

#endif // TABLEOFELEMENTS_H
