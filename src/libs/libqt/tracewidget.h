// This is a -*- C++ -*- header.
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#ifndef TRACEWIDGET_H
#define TRACEWIDGET_H

#include <QWidget>

namespace libqt {
  
  struct TraceWidgetData;
  
  class TraceWidget : public QWidget {
    Q_OBJECT
      ;
  public:
    ~TraceWidget();
    explicit TraceWidget(QWidget *parent = 0);
    
  signals:
    
  public slots:
    
  private:
    TraceWidgetData * d_;
  };

}

#endif // TRACEWIDGET_H
