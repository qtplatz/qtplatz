//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "dataplotwidget.h"

using namespace adil::ui;

DataplotWidget::DataplotWidget(QWidget *parent) :  Dataplot(parent)
                                                , trackLeft_(true)
												, trackRight_(true) 
												, allowYZoom_(true)
												, autoYScale_(false) 
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
	switch( button ) {
    case 0:
		if ( trackLeft_ )
			emit NotifyLButtonDown( x, y, false, false );
		break;
	case 1:
        emit NotifyMButtonDown( x, y, false, false );
		break;
	case 2:
        if ( trackRight_ )
			emit NotifyRButtonDown( x, y, false, false );
		break;
	default:
		break;
	};
	//emit NotifyMouseDown(x, y, button );
}

void
DataplotWidget::OnMouseUp( double x, double y, short button )
{
	switch ( button )	{
	default :
	case 0 :
		break;
		
	case 1 :
		if ( trackLeft_ )
			emit NotifyLButtonUp( x, y, false, false );
		break;
	case 2 :
		emit NotifyMButtonUp( x, y, false, false );
		break;
	case 3 :
		if ( trackRight_ )
			emit NotifyRButtonUp( x, y, false, false );
		break;
	}
/**
	mbCapture = FALSE;
	LinkedViews::iterator it;
	for (it = mLinks.begin(); it != mLinks.end(); it++)
		(*it)->mbCapture = FALSE;
*/
	//emit NotifyMouseUp(x, y, button );
}

void
DataplotWidget::OnMouseMove( double x, double y, short button )
{
	switch ( button )	{
	default :
	case 0 :
		break;
		
	case 1 :
		if ( trackLeft_ )
			emit NotifyLButtonMove( x, y );
		break;
		
	case 2 :
		emit NotifyMButtonMove( x, y );
		break;
		
	case 3 :
		if ( trackRight_ )
			emit NotifyRButtonMove( x, y );
		break;
	}

	//bool bShift, bControl;
	//GetKeys(bShift, bControl);
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
  emit NotifyMouseDblClk(x, y, button );
}

