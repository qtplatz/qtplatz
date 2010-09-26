//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include <atlbase.h>
#include <atlcom.h>

#include "dataplotimpl.h"
#include "dataplot.h"
#include <QResizeEvent>
#include <QAxWidget>  // Fix me, this module requre commercial license
#include <QUuid>

#include "import_sagraphics.h"

#define QCLSID_SADataplot "{1033423F-6431-46CD-9824-C1A9CAE5861E}"
static QUuid QIID_ISADataplot(0x9bda62de,0x514e,0x4ffb,0x8d,0xcc,0xe1,0xa3,0x55,0xcf,0x6b,0xff);

using namespace adwidgets::ui::internal::win32;

DataplotImpl::DataplotImpl( adwidgets::ui::Dataplot& dataplot ) : dataplot_( dataplot )
                                                 , QAxWidget( &dataplot )
{
}

DataplotImpl::~DataplotImpl()
{
    using namespace SAGRAPHICSLib;
	HRESULT hr;
	hr = IDispEventSimpleImpl<100, DataplotImpl, &DIID__ISADataplotEvents>::DispEventUnadvise( pi_ );
	ATLASSERT( hr == S_OK);
	hr = IDispEventSimpleImpl<100, DataplotImpl, &DIID__ISADataplotEvents2>::DispEventUnadvise( pi_ );
    ATLASSERT( hr == S_OK );
}

static long default_trace_color_table[] = {
	// RGB( 240, 240, 240 ), // background
   RGB( 0, 0, 255), // blue
   RGB( 0, 255, 0), // green
   RGB( 0, 0, 255), // blue
   RGB( 0, 255, 255 ), // cyan
   RGB( 255, 0, 0 ), // red
   RGB( 255, 0, 255 ), // magenta
   RGB( 200, 200, 0 ),  // yellow
   RGB( 16, 16, 16 ),
};

bool
DataplotImpl::createControl()
{
    using namespace SAGRAPHICSLib;
	if ( this->setControl( QCLSID_SADataplot ) ) {
		pi_.Release();
		if ( this->queryInterface( QIID_ISADataplot, reinterpret_cast<void **>(&pi_) ) == S_OK ) {
            HRESULT hr;
            hr = IDispEventSimpleImpl<100, DataplotImpl, &DIID__ISADataplotEvents>::DispEventAdvise( pi_ );
            ATLASSERT( hr == S_OK );
            hr = IDispEventSimpleImpl<100, DataplotImpl, &DIID__ISADataplotEvents2>::DispEventAdvise( pi_ );
            ATLASSERT( hr == S_OK );
			this->activateWindow();

            SAGRAPHICSLib::ISADPColorsPtr colors = pi_->Colors;
            const int nColors = sizeof(default_trace_color_table)/sizeof(default_trace_color_table[0]);
            for ( int i = 0; i < nColors; ++i ) {
				SAGRAPHICSLib::ISADPColorPtr color = colors->GetItem( i + 1 );
				color->Value = COLORREF( default_trace_color_table[i] );
            }
			return true;
		}
	}
	return false;
}

STDMETHODIMP
DataplotImpl::OnMouseDown( double x, double y, short button )
{
    dataplot_.emitOnMouseDown( x, y, button );
	return S_OK;
}

STDMETHODIMP
DataplotImpl::OnMouseUp( double x, double y, short button )
{
    dataplot_.emitOnMouseUp( x, y, button );
	return S_OK;
}

STDMETHODIMP
DataplotImpl::OnMouseMove( double x, double y, short button )
{
    dataplot_.emitOnMouseMove( x, y, button );
	return S_OK;
}

STDMETHODIMP
DataplotImpl::OnCharacter( long KeyCode )
{
    dataplot_.emitOnCharacter( KeyCode );
	return S_OK;
}

STDMETHODIMP
DataplotImpl::OnKeyDown( long KeyCode )
{
    dataplot_.emitOnKeyDown( KeyCode );
	return S_OK;
}

STDMETHODIMP
DataplotImpl::OnSetFocus( long hWnd )
{
    dataplot_.emitOnSetFocus( hWnd );
	return S_OK;
}

STDMETHODIMP
DataplotImpl::OnKillFocus( long hWnd )
{
    dataplot_.emitOnKillFocus( hWnd );
	return S_OK;
}

STDMETHODIMP
DataplotImpl::OnMouseDblClk(double x, double y, short button )
{
    dataplot_.emitOnMouseDblClk( x, y, button );
	return S_OK;
}
