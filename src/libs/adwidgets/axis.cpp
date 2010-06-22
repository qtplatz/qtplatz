//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "axis.h"

#include "import_sagraphics.h"

using namespace adil::ui;

Axis::~Axis()
{
  if ( pi_ )
	  pi_->Release();
}

Axis::Axis( ISADPAxis * pi ) : pi_(pi)
{
	pi_->AddRef();
}

Axis::Axis( const Axis& axis )
{
   if ( axis.pi_ )
	   axis.pi_->AddRef(); // AddRef first, in order to avoid unexpected release when self assignment happens
   if ( pi_ )
	   pi_->Release();
   pi_ = axis.pi_;
}


bool
Axis::visible() const
{
	if ( pi_ ) {
        VARIANT_BOOL v;
		if ( pi_->get_Visible( &v ) == S_OK )
			return v == 0 ? false : true;
	}
	return false;
}

void
Axis::visible(bool newValue)
{
	VARIANT_BOOL v = newValue ? VARIANT_TRUE : VARIANT_FALSE;
    if ( pi_ )
		pi_->put_Visible( v );
}

double
Axis::minimum() const
{
	double res = 0;
	if ( pi_ )
		pi_->get_Minimum(&res);
	return res;
}

void
Axis::minimum(double newValue)
{
	if ( pi_ )
		pi_->put_Minimum(newValue);
}

double
Axis::maximum() const
{
	double res = 0;
	if ( pi_ )
		pi_->get_Maximum(&res);
	return res;
}

void
Axis::maximum(double newValue)
{
	if ( pi_ )
		pi_->put_Maximum(newValue);
}

double
Axis::zoomMinimum() const
{
	double res = 0;
    if ( pi_ )
		pi_->get_ZoomMinimum(&res);
	return res;
}

double
Axis::zoomMaximum() const
{
	double res = 0;
    if ( pi_ )
		pi_->get_ZoomMaximum(&res);
	return res;
}

long
Axis::tickComputeStyle() const
{
	TickComputeStyle res;
    if ( pi_ )
		pi_->get_TickComputeStyle(&res);
	return static_cast<long>(res);
}

void
Axis::tickComputeStyle(long newValue)
{
    if ( pi_ )
		pi_->put_TickComputeStyle(static_cast<TickComputeStyle>(newValue) );
}

double
Axis::majorTickIncrement() const
{
	double res = 0;
    if ( pi_ )
		pi_->get_MajorTickIncrement(&res);
	return res;
}

void
Axis::majorTickIncrement(double newValue)
{
    if ( pi_ )
		pi_->put_MajorTickIncrement( newValue );
}

double
Axis::minorTickIncrement() const
{
    double res = 0;
    if ( pi_ )
		pi_->get_MajorTickIncrement( & res );
	return res;
}

void
Axis::minorTickIncrement(double newValue)
{
   if ( pi_ )
	   pi_->put_MinorTickIncrement( newValue );
}

long
Axis::majorTickStyle() const
{
	TickStyle res;
   if ( pi_ )
	   pi_->get_MajorTickStyle(&res);
   return static_cast<long>(res);
}

void
Axis::majorTickStyle(long newValue)
{
	if ( pi_ )
		pi_->put_MajorTickStyle( static_cast<TickStyle>(newValue) );
}

long
Axis::minorTickStyle() const
{
	TickStyle res;
	if ( pi_ )
		pi_->get_MinorTickStyle( &res );
	return static_cast<long>(res);
}

void
Axis::minorTickStyle(long newValue)
{
	if ( pi_ )
		pi_->put_MinorTickStyle( static_cast<TickStyle>(newValue) );
}

long
Axis::tickLabelFormat() const
{
	TickLabelFormat res;
    if ( pi_ )
		pi_->get_TickLabelFormat(&res);
	return res;
}

void
Axis::tickLabelFormat(long newValue)
{
    if ( pi_ )
		pi_->put_TickLabelFormat( static_cast<TickLabelFormat>(newValue) );
}

short
Axis::tickLabelDecimals() const
{
	short res = 0;
	if ( pi_ )
		pi_->get_TickLabelDecimals(&res);
	return res;
}

void
Axis::tickLabelDecimals(short newValue)
{
    if ( pi_ )
		pi_->put_TickLabelDecimals( newValue );
}

unsigned long
Axis::color() const
{
	OLE_COLOR res = 0;
    if ( pi_ )
		pi_->get_Color(&res);
	return res;
}

void
Axis::color(unsigned long newValue)
{
   if ( pi_ )
	   pi_->put_Color( newValue );
}

std::wstring
Axis::text() const
{
	CComBSTR str;
	if ( pi_ )
		pi_->get_Text(&str);
	return std::wstring( static_cast<const wchar_t *>(str) );
}

void
Axis::text(const std::wstring& newValue)
{
    CComBSTR str( newValue.c_str() );
    if ( pi_ )
		pi_->put_Text( str );
}

// LPDISPATCH Axis::Font()
long
Axis::orientation() const
{
	Orientation res;
    if ( pi_ )
		pi_->get_Orientation(&res);
	return res;
}

long
Axis::scaleStyle() const
{
	ScaleStyle res;
	if ( pi_ )
		pi_->get_ScaleStyle(&res);
	return res;
}

void
Axis::scaleStyle(long newValue)
{
	if ( pi_ )
		pi_->put_ScaleStyle( static_cast<ScaleStyle>(newValue) );
}

//LPDISPATCH Axis::Ticks()
double
Axis::firstTickValue() const
{
	double res = 0;
	if ( pi_ )
		pi_->get_FirstTickValue(&res);
	return res;
}

void
Axis::firstTickValue(double newValue)
{
	if ( pi_ )
		pi_->put_FirstTickValue( newValue );
}

long
Axis::labelOrientation() const
{
	Orientation res;
	if ( pi_ )
		pi_->get_LabelOrientation(&res);
	return res;
}

void
Axis::labelOrientation(long newValue)
{
    if ( pi_ )
		pi_->put_LabelOrientation( static_cast<Orientation>(newValue) );
}

short
Axis::lineWidth() const
{
   short res = 0;
   if ( pi_ )
	   pi_->get_LineWidth( &res );
   return res;
}

void
Axis::lineWidth(short newValue)
{
    if ( pi_ )
		pi_->put_LineWidth( newValue );
}

long
Axis::lineStyle() const
{
	LineStyle res;
	if ( pi_ )
		pi_->get_LineStyle(&res);
	return res;
}

void
Axis::lineStyle(long newValue)
{
	if ( pi_ )
		pi_->put_LineStyle( static_cast<LineStyle>(newValue) );
}

bool
Axis::enableMarker() const
{
	VARIANT_BOOL res;
	if ( pi_ )
		pi_->get_EnableMarker(&res);
	return res == VARIANT_FALSE ? false : true;
}

void
Axis::enableMarker(bool newValue)
{
	if ( pi_ )
		pi_->put_EnableMarker( newValue ? VARIANT_TRUE : VARIANT_FALSE );
}

double
Axis::markerPosition() const
{
	double res = 0;
	if ( pi_ )
		pi_->get_MarkerPosition(&res);
	return res;
}

void
Axis::markerPosition(double newValue)
{
   if ( pi_ )
	   pi_->put_MarkerPosition( newValue );
}

short
Axis::tickLabelMaximumLength() const
{
	short res = 0;
    if ( pi_ )
        pi_->get_TickLabelMaximumLength(&res);
	return res;
}

void
Axis::tickLabelMaximumLength(short newValue)
{
	if ( pi_ )
		pi_->put_TickLabelMaximumLength( newValue );
}

bool
Axis::tickLabelsVisible() const
{
	VARIANT_BOOL res = VARIANT_FALSE;
    if ( pi_ )
		pi_->get_TickLabelsVisible( &res );
	return res == VARIANT_FALSE ? false : true;
}

void
Axis::tickLabelsVisible(bool newValue)
{
	if ( pi_ )
		pi_->put_TickLabelsVisible( newValue ? VARIANT_TRUE : VARIANT_FALSE );
}

