// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef DATAPLOTWIDGET_H
#define DATAPLOTWIDGET_H

#include "dataplot.h"
#include <boost/smart_ptr.hpp>
#include <boost/noncopyable.hpp>

namespace adil {
  namespace ui {

    namespace internal {
      class DataplotWidgetImpl;
    }

    class DataplotWidget : public Dataplot {
      Q_OBJECT
    public:
        virtual ~DataplotWidget();
      explicit DataplotWidget(QWidget *parent = 0);
      enum { eVK_SHIFT
           , eVK_CONTROL 
           , eVK_MENU 
      };

    private:
      bool init();
      
    public:

      signals:
      void NotifyLButtonDown(double x, double y, bool bShift, bool bContrl );
      void NotifyMButtonDown(double x, double y, bool bShift, bool bContrl );
      void NotifyRButtonDown(double x, double y, bool bShift, bool bContrl );
      void NotifyLButtonUp(double x, double y, bool bShift, bool bContrl );
      void NotifyMButtonUp(double x, double y, bool bShift, bool bContrl );
      void NotifyRButtonUp(double x, double y, bool bShift, bool bContrl );
      void NotifyLButtonMove(double x, double y);
      void NotifyMButtonMove(double x, double y);
      void NotifyRButtonMove(double x, double y);
      
      protected
      slots:
      // Dataplot class
      virtual void OnMouseDown( double x, double y, short button );
      virtual void OnMouseUp( double x, double y, short Button );
      virtual void OnMouseMove( double x, double y, short Button );
      virtual void OnCharacter( long KeyCode );
      virtual void OnKeyDown( long KeyCode );
      virtual void OnSetFocus( long hWnd );
      virtual void OnKillFocus( long hWnd );
      virtual void OnMouseDblClk( double x, double y, short button );
    private:
        //boost::scoped_ptr<internal::DataplotWidgetImpl> pImpl_;
        internal::DataplotWidgetImpl * pImpl_;
    };

  }
}

#endif // DATAPLOTWIDGET_H
