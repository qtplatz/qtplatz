// This is a -*- C++ -*- header.
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#ifndef ELEMENTALCOMPWND_H
#define ELEMENTALCOMPWND_H

#include <QWidget>
#include <boost/shared_ptr.hpp>

namespace Analysis {
  namespace internal {
    
    class ElementalCompWndImpl;
    
    class ElementalCompWnd : public QWidget {
      Q_OBJECT
	public:
      explicit ElementalCompWnd(QWidget *parent = 0);
      void init();
      
    signals:
      
      public slots:
      
    private:
      boost::shared_ptr<ElementalCompWndImpl> pImpl_;
      
    };
    
    /////////
  }
}

#endif // ELEMENTALCOMPWND_H
