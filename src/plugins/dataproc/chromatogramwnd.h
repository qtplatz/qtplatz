// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#ifndef CHROMATOGRAMWND_H
#define CHROMATOGRAMWND_H

#include <QWidget>
#include <boost/shared_ptr.hpp>

namespace dataproc {
  namespace internal {

    class ChromatogramWndImpl;

    class ChromatogramWnd : public QWidget {
      Q_OBJECT
	public:
      explicit ChromatogramWnd(QWidget *parent = 0);
      void init();
      
    signals:
      
      public slots:

      private:
        boost::shared_ptr<ChromatogramWndImpl> pImpl_;

    };
  }
}
#endif // CHROMATOGRAMWND_H
