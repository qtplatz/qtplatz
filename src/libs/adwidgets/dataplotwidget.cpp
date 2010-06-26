//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "dataplotwidget.h"
#include "import_sagraphics.h"
#include "axis.h"
#include <bitset>
#include <cstdlib>

using namespace adil::ui;

namespace adil {
  namespace ui {
    namespace internal {

      class DataplotWidgetImpl {
      public:
          ~DataplotWidgetImpl();
          DataplotWidgetImpl( DataplotWidget& );

          void OnMouseDown( double x, double y, short button );
          void OnMouseUp( double x, double y, short Button );
          void OnMouseMove( double x, double y, short Button );
          void OnMouseDblClk( double x, double y, short button );
      private:
          void OnNullButton( double x, double y );
          void OnLButtonDown( double x, double y );
          void OnMButtonDown( double x, double y );
          void OnRButtonDown( double x, double y );
          void OnLButtonUp( double x, double y );
          void OnMButtonUp( double x, double y );
          void OnRButtonUp( double x, double y );
          void OnLButtonMove( double x, double y );
          void OnMButtonMove( double x, double y );
          void OnRButtonMove( double x, double y );
          void OnLButtonDblClk( double x, double y );
          void OnMButtonDblClk( double x, double y );
          void OnRButtonDblClk( double x, double y );

          DataplotWidget& widget_;
          bool bCapture_;
          POINT capturePt_;
          std::pair<double, double> maxX_; // low, high
          std::pair<double, double> maxY_; // low, high
          std::pair<double, double> captureXY_;
          bool bAutoYZoom_;
          bool trackLeft_;
          bool trackRight_;
      };
    }
  }
}

//////////////////////////////////////////////////////////////////
DataplotWidget::~DataplotWidget()
{
    delete pImpl_;
}

DataplotWidget::DataplotWidget(QWidget *parent) : Dataplot(parent)
						, pImpl_( new internal::DataplotWidgetImpl(*this) )
{
}

bool
DataplotWidget::init()
{
  return true;
}

void
DataplotWidget::OnMouseDown(double x, double y, short button )
{
    pImpl_->OnMouseDown(x, y, button);
}

void
DataplotWidget::OnMouseUp( double x, double y, short button )
{
    pImpl_->OnMouseUp( x, y, button);
}

void
DataplotWidget::OnMouseMove( double x, double y, short button )
{
    pImpl_->OnMouseMove( x, y, button);
}

void
DataplotWidget::OnCharacter( long KeyCode )
{
    emit NotifyCharacter( KeyCode );
}

void
DataplotWidget::OnKeyDown( long KeyCode )
{
    emit NotifyKeyDown( KeyCode );
}

void
DataplotWidget::OnSetFocus( long hWnd )
{
    emit NotifySetFocus( hWnd );
}

void
DataplotWidget::OnKillFocus( long hWnd )
{
    emit NotifyKillFocus( hWnd );
}

void
DataplotWidget::OnMouseDblClk(double x, double y, short button )
{
    pImpl_->OnMouseDblClk( x, y, button );
}

///////////////////////////////

internal::DataplotWidgetImpl::~DataplotWidgetImpl()
{
}

internal::DataplotWidgetImpl::DataplotWidgetImpl( DataplotWidget& w) : widget_(w)
                                                                     , captureXY_(0, 0)
                                                                     , bAutoYZoom_(false)
                                                                     , maxX_(0, 100)
                                                                     , maxY_(0, 100)
{
}
  
void
internal::DataplotWidgetImpl::OnMouseDown( double x, double y, short button )
{
	static void (internal::DataplotWidgetImpl::*fptr[])(double x, double y) = {
		&internal::DataplotWidgetImpl::OnNullButton,  // 0
		&internal::DataplotWidgetImpl::OnLButtonDown, // 1
		&internal::DataplotWidgetImpl::OnMButtonDown, // 2
		&internal::DataplotWidgetImpl::OnRButtonDown, // 3
	};
    bCapture_ = true;
    ::GetCursorPos( &capturePt_ );
    (this->*fptr[button&07])( x, y );
}

void
internal::DataplotWidgetImpl::OnMouseUp( double x, double y, short button )
{
	static void (internal::DataplotWidgetImpl::*fptr[])(double x, double y) = {
		&internal::DataplotWidgetImpl::OnNullButton,  // 0
		&internal::DataplotWidgetImpl::OnLButtonUp, // 1
		&internal::DataplotWidgetImpl::OnMButtonUp, // 2
		&internal::DataplotWidgetImpl::OnRButtonUp, // 3
	};
	(this->*fptr[button&07])( x, y );
    bCapture_ = false;
}

void
internal::DataplotWidgetImpl::OnMouseMove( double x, double y, short button )
{
    void (internal::DataplotWidgetImpl::*fptr[])(double x, double y) = {
        &internal::DataplotWidgetImpl::OnNullButton,  // 0
        &internal::DataplotWidgetImpl::OnLButtonMove, // 1
        &internal::DataplotWidgetImpl::OnMButtonMove, // 2
        &internal::DataplotWidgetImpl::OnRButtonMove, // 3
    };
    (this->*fptr[button&07])( x, y );
}
  
void
internal::DataplotWidgetImpl::OnMouseDblClk( double x, double y, short button )
{
    void (internal::DataplotWidgetImpl::*fptr[])(double x, double y) = {
        &internal::DataplotWidgetImpl::OnNullButton,  // 0
        &internal::DataplotWidgetImpl::OnLButtonDblClk, // 1
        &internal::DataplotWidgetImpl::OnMButtonDblClk, // 2
        &internal::DataplotWidgetImpl::OnRButtonDblClk, // 3
    };
    (this->*fptr[button&07])( x, y );
}

////
void
internal::DataplotWidgetImpl::OnNullButton( double x, double y )
{
    Q_UNUSED(x);
    Q_UNUSED(y);
}

void
internal::DataplotWidgetImpl::OnLButtonDown( double x, double y )
{
    captureXY_ = std::pair<double, double>(x, y);
    widget_.cursorStyle( SAGRAPHICSLib::CS_Box );
    Axis axis = widget_.axisX();
    axis.enableMarker( true );
    axis.markerPosition( x );

    // check only 'key is down' and ignore toggle (caps lock)
    unsigned short lshift = ::GetKeyState( VK_LSHIFT ) & 0x80;
    unsigned short rshift = ::GetKeyState( VK_RSHIFT ) & 0x80;
    unsigned short lctrl  = ::GetKeyState( VK_LCONTROL ) & 0x80;
    unsigned short rctrl  = ::GetKeyState( VK_RCONTROL ) & 0x80;

    std::bitset<16> keyState( ((lctrl | rctrl) << DataplotWidget::eVK_CONTROL) | 
        ((lshift|rshift) << DataplotWidget::eVK_SHIFT) );
}

void
internal::DataplotWidgetImpl::OnMButtonDown( double x, double y )
{
    Q_UNUSED(x);
    Q_UNUSED(y);
}

void
internal::DataplotWidgetImpl::OnRButtonDown( double x, double y )
{
    Q_UNUSED(x);
    Q_UNUSED(y);
}

void
internal::DataplotWidgetImpl::OnLButtonUp( double x, double y )
{
    SAGRAPHICSLib::CursorStyle style = static_cast<SAGRAPHICSLib::CursorStyle>(widget_.cursorStyle());
    widget_.cursorStyle( SAGRAPHICSLib::CS_Plus );

    widget_.axisX().enableMarker( false );

    if ( bCapture_ ) {
        POINT pt;
        ::GetCursorPos(&pt);
        long dx = std::abs( capturePt_.x - pt.x );
        long dy = std::abs( capturePt_.y - pt.y );
        bool zoomX(false);
        bool zoomY(false);
        std::pair<double, double> xrange( maxX_ );
        std::pair<double, double> yrange( maxY_ );

        if ( dx > 4 ) {
            zoomX = true;
            if ( x > captureXY_.first )
                xrange = std::pair<double, double>( captureXY_.first, x );
            else
                xrange = std::pair<double, double>( x, captureXY_.first );
        }
        if ( dy > 4 ) {
            zoomY = true;
            if ( y > captureXY_.second )
                yrange = std::pair<double, double>( captureXY_.second, y );
            else
                yrange = std::pair<double, double>( y, captureXY_.second );
        }
        if ( bAutoYZoom_ && zoomX ) {
            widget_.zoomXAutoscaleY( xrange.first, xrange.second );
        } else {
            widget_.zoomXY( xrange.first, yrange.first, xrange.second, yrange.first );
        }
    }
}

void
internal::DataplotWidgetImpl::OnMButtonUp( double x, double y )
{
    Q_UNUSED(x);
    Q_UNUSED(y);
}

void
internal::DataplotWidgetImpl::OnRButtonUp( double x, double y )
{
    Q_UNUSED(x);
    Q_UNUSED(y);
}

void
internal::DataplotWidgetImpl::OnLButtonMove( double x, double y )
{
    Q_UNUSED(y);
    if ( bCapture_ ) {
        widget_.axisX().markerPosition( x );
        POINT pt;
        ::GetCursorPos(&pt);
        int dx = std::abs( pt.x - capturePt_.x );
        int dy = std::abs( pt.y - capturePt_.y );

        if ( dy < 8 )
            widget_.cursorStyle( SAGRAPHICSLib::CS_HorizontalLine );
        else if ( dx < 8 )
            widget_.cursorStyle( SAGRAPHICSLib::CS_VerticalLine );
        else
            widget_.cursorStyle( SAGRAPHICSLib::CS_Box );
    }
}

void
internal::DataplotWidgetImpl::OnMButtonMove( double x, double y )
{
    Q_UNUSED(x);
    Q_UNUSED(y);
}

void
internal::DataplotWidgetImpl::OnRButtonMove( double x, double y )
{
    Q_UNUSED(x);
    Q_UNUSED(y);
}

void
internal::DataplotWidgetImpl::OnLButtonDblClk( double x, double y )
{
    Q_UNUSED(x);
    Q_UNUSED(y);
    widget_.zoomOut();
}

void
internal::DataplotWidgetImpl::OnMButtonDblClk( double x, double y )
{
    Q_UNUSED(x);
    Q_UNUSED(y);
}

void
internal::DataplotWidgetImpl::OnRButtonDblClk( double x, double y )
{
    Q_UNUSED(x);
    Q_UNUSED(y);
}

