//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "dataplot.h"

#ifdef WIN32
#include <atlbase.h>
#include <atlcom.h>

#include <QAxWidget>  // Fix me, this module requre commercial license
#include <QUuid>
#endif

#include <QResizeEvent>

#include "axis.h"
#include "title.h"
#include "titles.h"
#include "trace.h"
#include "traces.h"
#include "colors.h"
#include "legend.h"

#define CLSID_SADataplot "{1033423F-6431-46CD-9824-C1A9CAE5861E}"
static QUuid QIID_ISADataplot(0x9bda62de,0x514e,0x4ffb,0x8d,0xcc,0xe1,0xa3,0x55,0xcf,0x6b,0xff);

#include "import_sagraphics.h"

using namespace adil;
using namespace adil::ui;

namespace adil {
	namespace ui {
		struct DataplotImpl : QAxWidget {
			DataplotImpl( QWidget * parent = 0 ) : QAxWidget( parent ) {}
			CComPtr<ISADataplot> plot_;
		};
	}
}

Dataplot::~Dataplot()
{
	delete pImpl_;
}

Dataplot::Dataplot(QWidget *parent) : QWidget(parent)
                                    , pImpl_(0)
{
	if ( pImpl_ = new DataplotImpl(this) ) {
		createControl();
	}
}

bool
Dataplot::createControl()
{
	if ( pImpl_ && pImpl_->setControl( CLSID_SADataplot ) ) {
		CComPtr<IDispatch> idisp;
		if ( pImpl_->queryInterface( IID_IDispatch, reinterpret_cast<void **>(&idisp) ) == S_OK ) {
			pImpl_->plot_ = idisp;
			pImpl_->activateWindow();
			return true;
		}
	}
	return false;
}

Axis
Dataplot::axisX()
{
	if ( pImpl_ && pImpl_->plot_ ) {
		CComPtr<ISADPAxis> piAxis;
		pImpl_->plot_->get_AxisX( &piAxis );
		return Axis( piAxis );
	}
	return Axis();
}

Axis
Dataplot::axisY()
{
	if ( pImpl_ && pImpl_->plot_ ) {
		CComPtr<ISADPAxis> piAxis;
		pImpl_->plot_->get_AxisY( &piAxis );
		return Axis( piAxis );
	}
	return Axis();
}

Axis
Dataplot::axisY1()
{
	if ( pImpl_ && pImpl_->plot_ ) {
		CComPtr<ISADPAxis> piAxis;
		pImpl_->plot_->get_AxisY1( &piAxis );
		return Axis( piAxis );
	}
	return Axis();
}

Axis
Dataplot::axisY2()
{
	if ( pImpl_ && pImpl_->plot_ ) {
		CComPtr<ISADPAxis> piAxis;
		pImpl_->plot_->get_AxisY2( &piAxis );
		return Axis( piAxis );
	}
	return Axis();
}

void
Dataplot::resizeEvent(QResizeEvent * e)
{
	if ( pImpl_ )
		pImpl_->resize( e->size() );
}