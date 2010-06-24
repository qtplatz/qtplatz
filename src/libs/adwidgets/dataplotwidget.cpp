//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "dataplotwidget.h"
#include "import_sagraphics.h"
#include "axis.h"

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
	std::pair<double, double>  captureXY_;
	bool trackLeft_;
	bool trackRight_;
      };
    }
  }
}

//////////////////////////////////////////////////////////////////

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
								     , trackLeft_(true)
								     , trackRight_(true)
								     , captureXY_(0, 0)
{
}
  
void
internal::DataplotWidgetImpl::OnMouseDown( double x, double y, short button )
{
  void (internal::DataplotWidgetImpl::*fptr[])(double x, double y) = {
    &internal::DataplotWidgetImpl::OnNullButton,  // 0
    &internal::DataplotWidgetImpl::OnLButtonDown, // 1
    &internal::DataplotWidgetImpl::OnMButtonDown, // 2
    &internal::DataplotWidgetImpl::OnRButtonDown, // 3
  };
  (this->*fptr[button&07])( x, y );
}

void
internal::DataplotWidgetImpl::OnMouseUp( double x, double y, short button )
{
  void (internal::DataplotWidgetImpl::*fptr[])(double x, double y) = {
    &internal::DataplotWidgetImpl::OnNullButton,  // 0
    &internal::DataplotWidgetImpl::OnLButtonUp, // 1
    &internal::DataplotWidgetImpl::OnMButtonUp, // 2
    &internal::DataplotWidgetImpl::OnRButtonUp, // 3
  };
  (this->*fptr[button&07])( x, y );
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
}

void
internal::DataplotWidgetImpl::OnLButtonDown( double x, double y )
{
  captureXY_ = std::pair<double, double>(x, y);
  widget_.cursorStyle( CS_HorizontalLine );
  Axis axis = widget_.axisX();
  axis.enableMarker( true );
  axis.markerPosition( x );
}

void
internal::DataplotWidgetImpl::OnMButtonDown( double x, double y )
{
}

void
internal::DataplotWidgetImpl::OnRButtonDown( double x, double y )
{
}

void
internal::DataplotWidgetImpl::OnLButtonUp( double x, double y )
{
}

void
internal::DataplotWidgetImpl::OnMButtonUp( double x, double y )
{
}

void
internal::DataplotWidgetImpl::OnRButtonUp( double x, double y )
{
}

void
internal::DataplotWidgetImpl::OnLButtonMove( double x, double y )
{
}

void
internal::DataplotWidgetImpl::OnMButtonMove( double x, double y )
{
}

void
internal::DataplotWidgetImpl::OnRButtonMove( double x, double y )
{
}

void
internal::DataplotWidgetImpl::OnLButtonDblClk( double x, double y )
{
}

void
internal::DataplotWidgetImpl::OnMButtonDblClk( double x, double y )
{
}

void
internal::DataplotWidgetImpl::OnRButtonDblClk( double x, double y )
{
}

