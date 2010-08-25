//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "dataplot.h"

//#ifdef WIN32
//#include <atlbase.h>
//#include <atlcom.h>
//#include <QAxWidget>  // Fix me, this module requre commercial license
//#include <QUuid>
//#endif

#include <QResizeEvent>

#include "axis.h"
#include "title.h"
#include "titles.h"
#include "trace.h"
#include "traces.h"
#include "colors.h"
#include "legend.h"
#include "plotregion.h"

//#define QCLSID_SADataplot "{1033423F-6431-46CD-9824-C1A9CAE5861E}"
//static QUuid QIID_ISADataplot(0x9bda62de,0x514e,0x4ffb,0x8d,0xcc,0xe1,0xa3,0x55,0xcf,0x6b,0xff);

#include "dataplotimpl.h"
#include "import_sagraphics.h"

using namespace adwidgets;
using namespace adwidgets::ui;

Dataplot::~Dataplot()
{
}

Dataplot::Dataplot(QWidget *parent) : QWidget(parent)
{
  // Only support win32 COM implementation so far.
  using namespace internal;
  pImpl_.reset( new win32::DataplotImpl(*this) );
  if ( pImpl_ )
    createControl();
}

bool
Dataplot::createControl()
{
  if ( pImpl_ ) 
    return pImpl_->createControl();
  return false;
}

void
Dataplot::resizeEvent(QResizeEvent * e)
{
  if ( pImpl_ )
    pImpl_->resize( e->size() );
}

///////////////////////////////////////////////////////
Axis
Dataplot::axisX() const
{
	CComPtr<SAGRAPHICSLib::ISADPAxis> piAxis;
	(*pImpl_)->get_AxisX( &piAxis );
    return Axis( piAxis );
}

Axis
Dataplot::axisY() const
{
	CComPtr<SAGRAPHICSLib::ISADPAxis> piAxis;
	(*pImpl_)->get_AxisY( &piAxis );
	return Axis( piAxis );
}

Axis
Dataplot::axisY1() const
{
    CComPtr<SAGRAPHICSLib::ISADPAxis> piAxis;
    (*pImpl_)->get_AxisY1( &piAxis );
    return Axis( piAxis );
}

Axis
Dataplot::axisY2() const
{
	CComPtr<SAGRAPHICSLib::ISADPAxis> piAxis;
	(*pImpl_)->get_AxisY2( &piAxis );
	return Axis( piAxis );
}


void
Dataplot::autoSize(bool newValue)
{
	(*pImpl_)->put_AutoSize( newValue ? VARIANT_TRUE : VARIANT_FALSE );
}

bool 
Dataplot::autoSize() const
{
  VARIANT_BOOL result;
  (*pImpl_)->get_AutoSize(&result);
  return result == VARIANT_FALSE ? false : true;
}
void
Dataplot::backColor(unsigned long newValue)
{
  (*pImpl_)->put_BackColor( newValue );
}

unsigned long
Dataplot::backColor() const
{
  unsigned long ret;
  (*pImpl_)->get_BackColor(&ret);
  return ret;
}

void
Dataplot::borderColor(unsigned long newValue)
{
  (*pImpl_)->put_BorderColor( newValue );
}

unsigned long
Dataplot::borderColor() const
{
  unsigned long result;
  (*pImpl_)->get_BorderColor(&result);
  return result;
}

void
Dataplot::borderStyle(long newValue)
{
  (*pImpl_)->put_BorderStyle( newValue );
}

long
Dataplot::borderStyle() const
{
  long result;
  (*pImpl_)->get_BorderStyle(&result);
  return result;
}

void
Dataplot::borderWidth(long newValue)
{
  (*pImpl_)->put_BorderWidth( newValue );
}

long
Dataplot::borderWidth() const
{
  long result;
  (*pImpl_)->get_BorderWidth(&result);
  return result;
}

void
Dataplot::foreColor(unsigned long newValue)
{
  (*pImpl_)->put_ForeColor( newValue );
}

unsigned long
Dataplot::foreColor() const
{
  unsigned long result;
  (*pImpl_)->get_ForeColor( &result );
  return result;
}

void
Dataplot::enabled(bool newValue)
{
	(*pImpl_)->put_Enabled( internal::variant_bool::to_variant( newValue ) );
}

bool 
Dataplot::enabled() const
{
	VARIANT_BOOL result;
	(*pImpl_)->get_Enabled( &result );
	return internal::variant_bool::to_native( result );
}

void
Dataplot::borderVisible(bool newValue)
{
	(*pImpl_)->put_BorderVisible( internal::variant_bool::to_variant(newValue) );
}

bool
Dataplot::borderVisible() const
{
	VARIANT_BOOL result;
    (*pImpl_)->get_BorderVisible( &result );
	return internal::variant_bool::to_native( result );
}

Titles 
Dataplot::titles() const
{
	CComPtr<SAGRAPHICSLib::ISADPTitles> p;
	(*pImpl_)->get_Titles( &p );
	return Titles( p );
}

PlotRegion 
Dataplot::plotRegion() const
{
	CComPtr<SAGRAPHICSLib::ISADPPlotRegion> p;
	(*pImpl_)->get_PlotRegion( &p );
	return PlotRegion( p );
}

Traces 
Dataplot::traces() const
{
	CComPtr<SAGRAPHICSLib::ISADPTraces> p;
	(*pImpl_)->get_Traces( &p );
	return Traces( p );
}

Colors 
Dataplot::colors() const
{
	CComPtr<SAGRAPHICSLib::ISADPColors> p;
	(*pImpl_)->get_Colors(&p);
    return Colors( p );
}

bool
Dataplot::visible() const
{
	VARIANT_BOOL result;
    (*pImpl_)->get_Visible(&result);
	return internal::variant_bool::to_native( result );
}

void
Dataplot::visible(bool newValue)
{
	(*pImpl_)->put_Visible( internal::variant_bool::to_variant(newValue) );
}

bool
Dataplot::redrawEnabled() const
{
	VARIANT_BOOL result;
	(*pImpl_)->get_RedrawEnabled( &result );
	return internal::variant_bool::to_native( result );
}

void
Dataplot::redrawEnabled(bool newValue)
{
	(*pImpl_)->put_RedrawEnabled( internal::variant_bool::to_variant( newValue ) );
}

long
Dataplot::cursorStyle() const
{
    SAGRAPHICSLib::CursorStyle result;
	(*pImpl_)->get_CursorStyle(&result);
	return result;
}

void
Dataplot::cursorStyle(long newValue)
{
    (*pImpl_)->put_CursorStyle( static_cast<SAGRAPHICSLib::CursorStyle>(newValue) );	
}

bool
Dataplot::scaleMinimumY() const
{
	VARIANT_BOOL result;
    (*pImpl_)->get_ScaleMinimumY(&result);
	return internal::variant_bool::to_native( result );
}

void
Dataplot::scaleMinimumY(bool newValue)
{
	(*pImpl_)->put_ScaleMinimumY( internal::variant_bool::to_variant( newValue ) );
}

bool
Dataplot::showCursor() const
{
	VARIANT_BOOL result;
	(*pImpl_)->get_ShowCursor( &result );
	return internal::variant_bool::to_native( result );
}

void
Dataplot::showCursor(bool newValue)
{
	(*pImpl_)->put_ShowCursor( internal::variant_bool::to_variant( newValue ) );
}

void
Dataplot::zoomXY(double minimumX, double minimumY, double maximumX, double maximumY)
{
	(*pImpl_)->ZoomXY(minimumX, minimumY, maximumX, maximumY);
}

void
Dataplot::zoomXAutoscaleY(double minimumX, double maximumX)
{
	(*pImpl_)->ZoomXAutoscaleY( minimumX, maximumX );	
}

void
Dataplot::zoomOut()
{
	(*pImpl_)->ZoomOut();
}

void
Dataplot::highlightColor(unsigned long newValue)
{
	(*pImpl_)->put_HighlightColor( newValue );	
}

unsigned long
Dataplot::highlightColor() const
{
	unsigned long result;
    (*pImpl_)->get_HighlightColor(&result);
	return result;
}

bool 
Dataplot::highlight() const
{
	VARIANT_BOOL result;
    (*pImpl_)->get_Highlight(&result);
	return internal::variant_bool::to_native(result);
}

void
Dataplot::highlight( bool newValue )
{ 
	(*pImpl_)->put_Highlight( internal::variant_bool::to_variant( newValue ) );	
}

long
Dataplot::currentMouseXPixel() const
{
	long result;
    (*pImpl_)->get_CurrentMouseXPixel(&result);
	return result;
}

long
Dataplot::currentMouseYPixel() const
{
	long result;
    (*pImpl_)->get_CurrentMouseYPixel(&result);	
	return result;
}

bool
Dataplot::contourLegendVisible() const
{
	VARIANT_BOOL result;
    (*pImpl_)->get_ContourLegendVisible(&result);
	return internal::variant_bool::to_native(result);
}

void
Dataplot::contourLegendVisible(bool newValue)
{
	(*pImpl_)->put_ContourLegendVisible( internal::variant_bool::to_variant(newValue) );
}

long
Dataplot::plotRegionPadding() const
{
	long result;
    (*pImpl_)->get_PlotRegionPadding(&result);	
	return result;
}

void
Dataplot::plotRegionPadding(long newValue)
{
    (*pImpl_)->put_PlotRegionPadding( newValue );
}

bool
Dataplot::useBitmaps() const
{
	VARIANT_BOOL result;
    (*pImpl_)->get_UseBitmaps(&result);

	return internal::variant_bool::to_native(result);
}

void
Dataplot::useBitmaps(bool newValue)
{
	(*pImpl_)->put_UseBitmaps( internal::variant_bool::to_variant(newValue) );
}

long
Dataplot::worldToPixelX(double x)
{
	long result;
	(*pImpl_)->get_WorldToPixelX(x, &result);
	return result;
}

long
Dataplot::worldToPixelY(double y)
{
	long result;
	(*pImpl_)->get_WorldToPixelY(y, &result);
	return result;
}

long
Dataplot::worldToPixelY2(double y)
{
	long result;
	(*pImpl_)->get_WorldToPixelY2(y, &result);
	return result;
}

double
Dataplot::pixelToWorldX(long x)
{
	double result;
	(*pImpl_)->get_PixelToWorldX(x, &result);
	return result;
}

double
Dataplot::pixelToWorldY(long y)
{
	double result;
	(*pImpl_)->get_PixelToWorldY(y, &result);
	return result;
}

double
Dataplot::pixelToWorldY2(long y)
{
	double result;
	(*pImpl_)->get_PixelToWorldY2(y, &result);
	return result;
}

void
Dataplot::setMousePosition(double x, double y)
{
	(*pImpl_)->SetMousePosition(x, y);
	
}

void
Dataplot::setCursorPosition(double x, double y)
{
  (*pImpl_)->SetCursorPosition(x, y);
}

void
Dataplot::moveCursorPosition(double x, double y)
{
	(*pImpl_)->MoveCursorPosition(x, y);
}

Legend
Dataplot::legend() const
{
    CComPtr<SAGRAPHICSLib::ISADPLegend> p;
	(*pImpl_)->get_Legend( &p );
    return Legend( p );
}

//////////////////////////
void
Dataplot::OnMouseDown(double x, double y, short button )
{
  emit NotifyMouseDown(x, y, button );
}

void
Dataplot::OnMouseUp( double x, double y, short button )
{
  emit NotifyMouseUp(x, y, button );
}

void
Dataplot::OnMouseMove( double x, double y, short button )
{
  emit NotifyMouseMove(x, y, button );
}

void
Dataplot::OnCharacter( long KeyCode )
{
  emit NotifyCharacter( KeyCode );
}

void
Dataplot::OnKeyDown( long KeyCode )
{
  emit NotifyKeyDown( KeyCode );
}

void
Dataplot::OnSetFocus( long hWnd )
{
  emit NotifySetFocus( hWnd );
}

void
Dataplot::OnKillFocus( long hWnd )
{
  emit NotifyKillFocus( hWnd );
}

void
Dataplot::OnMouseDblClk(double x, double y, short button )
{
  emit NotifyMouseDblClk(x, y, button );
}
