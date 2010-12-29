//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "dataplotwidget.h"
#include "import_sagraphics.h"
#include "axis.h"
#include <bitset>
#include <cstdlib>

using namespace adwidgets::ui;

namespace adwidgets {
    namespace ui {
        namespace internal {

            class DataplotWidgetImpl {
            public:
                ~DataplotWidgetImpl();
                DataplotWidgetImpl( DataplotWidget& );

                const std::pair<double, double>& display_range_x() const { return maxX_; }
                const std::pair<double, double>& display_range_y1() const { return maxY1_;} 
                const std::pair<double, double>& display_range_y2() const { return maxY2_; }
                void display_range_x( const std::pair<double, double>& t ) { maxX_ = t; }
                void display_range_y1( const std::pair<double, double>& t ) { maxY1_ = t; }
                void display_range_y2( const std::pair<double, double>& t ) { maxY2_ = t; }

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

                friend DataplotWidget;
                DataplotWidget& widget_;
                bool bCapture_;
                POINT capturePt_;
                std::pair<double, double> maxX_; // low, high
                std::pair<double, double> maxY1_; // low, high
                std::pair<double, double> maxY2_; // low, high
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
    std::pair<double, double> range_x( axisX().zoomMinimum(), axisX().zoomMaximum() );
    std::pair<double, double> range_y( axisY().zoomMinimum(), axisY().zoomMaximum() );
    display_range_x( range_x );
    display_range_y( range_y );
}

bool
DataplotWidget::init()
{
    return true;
}

const std::pair<double, double>&
DataplotWidget::display_range_x() const
{
    return pImpl_->display_range_x();
}

const std::pair<double, double>&
DataplotWidget::display_range_y() const
{
    return pImpl_->display_range_y1();
}

void
DataplotWidget::display_range_x( const std::pair<double, double>& t )
{
    pImpl_->display_range_x( t );
    axisX().minimum( t.first );
    axisX().maximum( t.second );
}

void
DataplotWidget::display_range_y( const std::pair<double, double>& t )
{
    pImpl_->display_range_y1( t );
    axisY().minimum( t.first );
    axisY().maximum( t.second );
}

void
DataplotWidget::display_range_y2( const std::pair<double, double>& t )
{
    pImpl_->display_range_y2( t );
    axisY2().minimum( t.first );
    axisY2().maximum( t.second );
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
    (void)KeyCode;
}

void
DataplotWidget::OnKeyDown( long KeyCode )
{
    (void)KeyCode;
}

void
DataplotWidget::OnSetFocus( long hWnd )
{
    (void)hWnd;
}

void
DataplotWidget::OnKillFocus( long hWnd )
{
	(void)hWnd;
}

void
DataplotWidget::OnMouseDblClk(double x, double y, short button )
{
    pImpl_->OnMouseDblClk( x, y, button );
}

void
DataplotWidget::link( DataplotWidget * p )
{
    connect( this, SIGNAL( signalZoomXAutoscaleY(double, double) ), p, SLOT( handleZoomXAutoscaleY( double, double ) ) );
    connect( this, SIGNAL( signalZoomXY(double, double, double, double) ), p, SLOT( handleZoomXY( double, double, double, double ) ) );
}

void
DataplotWidget::unlink( DataplotWidget * p )
{
    disconnect( this, SIGNAL( signalZoomXAutoscaleY(double, double) ), p, SLOT( handleZoomXAutoscaleY( double, double ) ) );
    disconnect( this, SIGNAL( signalZoomXY(double, double, double, double) ), p, SLOT( handleZoomXY( double, double, double, double ) ) );
}

void
DataplotWidget::handleZoomXAutoscaleY( double x1, double x2 )
{
    pImpl_->widget_.zoomXAutoscaleY( x1, x2 );
}

void
DataplotWidget::handleZoomXY( double x1, double y1, double x2, double y2 )
{
    pImpl_->widget_.zoomXY( x1, y1, x2, y2 );
}

///////////////////////////////

internal::DataplotWidgetImpl::~DataplotWidgetImpl()
{
}

internal::DataplotWidgetImpl::DataplotWidgetImpl( DataplotWidget& w) : widget_(w)
                                                                     , captureXY_(0, 0)
                                                                     , bAutoYZoom_(false)
                                                                     , maxX_(0, 100)
                                                                     , maxY1_(0, 100)
																	 , maxY2_(0, 100)
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

    widget_.axisX().enableMarker( true );
    widget_.axisX().markerPosition( x );
    widget_.axisY().enableMarker( true );
    widget_.axisY().markerPosition( y );

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
	//SAGRAPHICSLib::CursorStyle style = static_cast<SAGRAPHICSLib::CursorStyle>(widget_.cursorStyle());
    widget_.cursorStyle( SAGRAPHICSLib::CS_Plus );

    widget_.axisX().enableMarker( false );
    widget_.axisY().enableMarker( false );

    if ( bCapture_ ) {
        POINT pt;
        ::GetCursorPos(&pt);
        long dx = std::abs( capturePt_.x - pt.x );
        long dy = std::abs( capturePt_.y - pt.y );
        bool zoomX(false);
        bool zoomY(false);
        std::pair<double, double> xrange( maxX_ );
        std::pair<double, double> yrange( maxY1_ );

        if ( dx > 8 ) {
            zoomX = true;
            if ( x > captureXY_.first )
                xrange = std::pair<double, double>( captureXY_.first, x );
            else
                xrange = std::pair<double, double>( x, captureXY_.first );
        }
        if ( dy > 8 ) {
            zoomY = true;
            if ( y > captureXY_.second )
                yrange = std::pair<double, double>( captureXY_.second, y );
            else
                yrange = std::pair<double, double>( y, captureXY_.second );
        }
        if ( bAutoYZoom_ && zoomX ) {
            widget_.zoomXAutoscaleY( xrange.first, xrange.second );
            emit widget_.signalZoomXAutoscaleY( xrange.first, xrange.second );
        } else {
            widget_.zoomXY( xrange.first, yrange.first, xrange.second, yrange.second );
            emit widget_.signalZoomXY( xrange.first, yrange.first, xrange.second, yrange.second );
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
        widget_.axisY().markerPosition( y );
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

