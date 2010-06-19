//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#include "tracewidget.h"
#include <boost/smart_ptr.hpp>

#include <QAxWidget>

using namespace libqt;

namespace libqt {
  struct TraceWidgetData {
    ~TraceWidgetData() {
    }
    TraceWidgetData() {
    }
    boost::scoped_ptr< QAxWidget > axWidget_;
  };
}

#define CLSID_InternetExplorer "{8856F961-340A-11D0-A96B-00C04FD705A2}"

TraceWidget::~TraceWidget()
{
  delete d_;
}

TraceWidget::TraceWidget(QWidget *parent) : QWidget(parent)
					  , d_(0)
{
  if ( d_ = new TraceWidgetData() ) {
    d_->axWidget_.reset( new QAxWidget(this) );
    d_->axWidget_->setControl( CLSID_InternetExplorer );    
  }
}

