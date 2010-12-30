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

struct COLOR_TABLE {
	LPWSTR		name_;	
	COLORREF	clr_;
};

static COLOR_TABLE trace_color_table[] = {
    {L"whitesmoke",		RGB(0xf5, 0xf5, 0xf5)},  //  0
    {L"blue",			RGB(0x00, 0x00, 0xff)},  //  1
    {L"green",			RGB(0x00, 0x80, 0x00)},  //  2
    {L"darkturquoise",  RGB(0x00, 0xce, 0xd1)},  //  3
    {L"red",			RGB(0xff, 0x00, 0x00)},  //  4
    {L"magenta",		RGB(0xff, 0x00, 0xff)},  //  5
    {L"darkorange",		RGB(0xff, 0x8c, 0x00)},  //  6
    {L"navy",			RGB(0x00, 0x00, 0x80)},  //  7
    {L"darkgreen",		RGB(0x00, 0x64, 0x00)},  //  8
    {L"cyan",			RGB(0x00, 0xff, 0xff)},  //  9
    {L"darkred",		RGB(0x8b, 0x00, 0x00)},  // 10
    {L"darkmagenta",    RGB(0x8b, 0x00, 0x8b)},  // 11
    {L"yellow",			RGB(0xff, 0xff, 0x00)},  // 12
    {L"cornflowerblue", RGB(0x64, 0x95, 0xed)},  // 13
    {L"darkseagreen",   RGB(0x8f, 0xbc, 0x8f)},  // 14
    {L"lightseagreen",  RGB(0x20, 0xb2, 0xaa)},  // 15
    {L"indianred",		RGB(0xcd, 0x5c, 0x5c)},  // 16
    {L"mediumorchid",   RGB(0xba, 0x55, 0xd3)},  // 17
    {L"chocolate",		RGB(0xd2, 0x69, 0x1e)}   // 18
    //--
    , {L"red",          RGB( 255,    0,    0) } // 0x01
    , {L"black",        RGB(   0,    0,    0) } // 0x02
    , {L"green",        RGB(   0,  128,    0) } // 0x03
    , {L"dardorange",   RGB(0xff, 0x8c, 0x00) } // 0x04
    , {L"deeppink",     RGB(0xff, 0x14, 0x93) } // 0x05 ( cluster target | deconvolution )
    , {L"tan",          RGB( 210,  180,  140) } // tan
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
            const int nColors = sizeof(trace_color_table)/sizeof(trace_color_table[0]);
            for ( int i = 1; i < nColors; ++i ) {
                SAGRAPHICSLib::ISADPColorPtr color = colors->GetItem( i );
                color->Value = COLORREF( trace_color_table[i].clr_ );
            }
            return true;
        }
    }
    return false;
}

size_t
DataplotImpl::getControlColorIndex() const
{
    return 18;
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
