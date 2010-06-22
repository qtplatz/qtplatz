//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#include "tracewidget.h"
#include <boost/smart_ptr.hpp>
#include <QAxWidget>
#include <QUuid>
#include <QSize>
#include <QResizeEvent>

#ifdef WIN32
#include <atlbase.h>
#include <atlcom.h>
#endif

using namespace adil;

#import "C:/MassCentre3.1/bin/SAGraphicsU.dll" no_namespace, named_guids, raw_interfaces_only
#define CLSID_SADataplot "{1033423F-6431-46CD-9824-C1A9CAE5861E}"
static QUuid QIID_ISADataplot(0x9bda62de,0x514e,0x4ffb,0x8d,0xcc,0xe1,0xa3,0x55,0xcf,0x6b,0xff);

namespace adil {

  struct TraceWidgetData {
    ~TraceWidgetData() { }
    TraceWidgetData() { }
    QAxWidget * axWidget_;
    CComPtr<ISADataplot> idataplot_;
  };

}


TraceWidget::~TraceWidget()
{
  delete d_;
}

TraceWidget::TraceWidget(QWidget *parent) : QWidget(parent)
					  , d_(0)
{
#ifdef WIN32
  // HWND hWnd = this->winId();
  
  if ( d_ = new TraceWidgetData() ) {
    d_->axWidget_ = new QAxWidget(this);
    d_->axWidget_->setControl( CLSID_SADataplot );
    d_->axWidget_->queryInterface( IID_ISADataplot, reinterpret_cast<void **>(&d_->idataplot_) );
    QSize size = this->size();
    size.setHeight( size.height() / 3 );
    size.setWidth( size.width() / 2 );
    d_->axWidget_->resize(size);
    d_->axWidget_->activateWindow();
  }
#endif
}

void
TraceWidget::resizeEvent(QResizeEvent * e)
{
	d_->axWidget_->resize( e->size() );
}