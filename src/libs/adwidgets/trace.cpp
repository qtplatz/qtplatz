//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "trace.h"
#include "import_sagraphics.h"
#include "annotations.h"
#include "fractions.h"
#include "markers.h"
#include "peaks.h"
#include "baselines.h"
#include "ranges.h"
#include "font.h"
#include "filledranges.h"

using namespace adil::ui;

Trace::~Trace()
{
  if ( pi_ )
    pi_->Release();
}

Trace::Trace( ISADPTrace * pi ) : pi_(pi)
{
  pi_->AddRef();
}

Trace::Trace( const Trace& t )
{
   if ( t.pi_ )
     t.pi_->AddRef(); // AddRef first, in order to avoid unexpected release when self assignment happens
   if ( pi_ )
     pi_->Release();
   pi_ = t.pi_;
}

Annotations
Trace::annotations() const
{
   CComPtr<ISADPAnnotations> p;
   pi_->get_Annotations( &p );
   return Annotations( p );
}

Fractions
Trace::fractions() const
{
   CComPtr<ISADPFractions> p;
   pi_->get_Fractions( &p );
   return Fractions( p );
}

Markers
Trace::markers() const
{
	CComPtr<ISADPMarkers> p;
	pi_->get_Markers( &p );
    return Markers( p );
}

Peaks
Trace::peaks() const
{
	CComPtr<ISADPPeaks> p;
	pi_->get_Peaks( &p );
    return Peaks( p );
}

Ranges
Trace::ranges() const
{
	CComPtr<ISADPRanges> p;
	pi_->get_Ranges( &p );
    return Ranges( p );
}

bool 
Trace::visible() const
{
	VARIANT_BOOL result;
	pi_->get_Visible(&result);
	return internal::variant_bool::to_native(result);
}

void
Trace::visible(bool newValue)
{
	pi_->put_Visible( internal::variant_bool::to_variant(newValue) );	
}

bool 
Trace::selected() const
{
	VARIANT_BOOL result;
	pi_->get_Selected(&result);
	return internal::variant_bool::to_native( result );
}

void
Trace::selected(bool newValue)
{
	pi_->put_Selected( internal::variant_bool::to_variant(newValue) );
}

long
Trace::YAxis() const
{
	::YAxis result;
	pi_->get_YAxis( &result );
	return result;
}

void
Trace::YAxis(long newValue)
{
	pi_->put_YAxis( static_cast<::YAxis>(newValue) );
}

double
Trace::offsetX() const
{
	double result;
	pi_->get_OffsetX( & result );
	return result;
}

void
Trace::offsetX(double newValue)
{
    pi_->put_OffsetX( newValue );
}

double
Trace::offsetY() const
{
	double result;
	pi_->get_OffsetY( & result );	
	return result;
}

void
Trace::offsetY(double newValue)
{
    pi_->put_OffsetY( newValue );
}

adil::ui::Font
Trace::font() const
{
	CComPtr<IDispatch> p;
	pi_->get_Font( &p );
	return adil::ui::Font( p );
}

long
Trace::traceStyle() const
{
	TraceStyle result;
    pi_->get_TraceStyle(&result);
	return result;
}

void
Trace::traceStyle(long newValue)
{
   pi_->put_TraceStyle( static_cast<TraceStyle>(newValue) );
}

long
Trace::lineStyle() const
{
	LineStyle result;
    pi_->get_LineStyle(&result);
	return result;
}

void
Trace::lineStyle(long newValue)
{
    pi_->put_LineStyle( static_cast<LineStyle>(newValue) );
}

long
Trace::annotationStyle() const
{
	AnnotationStyle result;
    pi_->get_AnnotationStyle( &result );
	return result;
}

void
Trace::annotationStyle(long newValue)
{
	pi_->put_AnnotationStyle( static_cast<AnnotationStyle>(newValue) );
}

bool
Trace::autoAnnotation() const
{
	VARIANT_BOOL result;
    pi_->get_AutoAnnotation(&result);
	return internal::variant_bool::to_native(result);
}

void
Trace::autoAnnotation(bool newValue)
{
	pi_->put_AutoAnnotation( internal::variant_bool::to_variant( newValue ) );	
}

short
Trace::colorIndex() const
{
	short result;
    pi_->get_ColorIndex(&result);
	return result;
}

void
Trace::colorIndex(short newValue)
{
   pi_->put_ColorIndex( newValue );
}

void
Trace::setColorIndices( const std::vector<short>& indices )
{
	SAFEARRAY * pArr;
    VARIANT v;
    VariantClear(&v);
	if ( pArr = ::SafeArrayCreateVector( VT_I2, 0, indices.size() ) ) {
         void * pData(0);
		 if (SUCCEEDED(SafeArrayAccessData(pArr, &pData))) {
			 memcpy(pData, &indices[0], sizeof(short) * indices.size() );
			 ::SafeArrayUnaccessData( pArr );
             v.vt = VT_ARRAY | VT_I2;
             v.parray = pArr;
			 pi_->SetColorIndices( v );
			 ::VariantClear(&v);
		 }
	}
}

std::vector<short>
Trace::colorIndices() const
{
	std::vector<short> result;
    CComPtr<ISADPColorIndices> p;
	if ( pi_->get_ColorIndices( &p ) == S_OK ) {
        long nSize;
		p->get_Count(&nSize);
		result.reserve( nSize );
		for ( int i = 0; i < nSize; ++i ) {
			CComPtr<ISADPColorIndex> pIndex;
			if ( p->get_Item( i + 1, &pIndex ) == S_OK ) {
                short value;
				pIndex->get_Value(&value);
				result.push_back( value );
			}
		}
	}
	return result;
}

double
Trace::x(long Index) const
{
	double result;
	pi_->get_x(Index, &result);
	return result;
}

void
Trace::x(long Index, double newValue)
{
    pi_->put_x(Index, newValue);
}

double
Trace::y(long Index) const
{
	double result;
	pi_->get_y(Index, &result);
	return result;
}

void
Trace::y(long Index, double newValue)
{
    pi_->put_y(Index, newValue);
}

long
Trace::pointCount() const
{
	long result;
	pi_->get_PointCount(&result);	
	return result;
}

void
Trace::clear()
{
	pi_->Clear();
}

Baselines
Trace::baselines() const
{
    CComPtr<ISADPBaselines> p;
    pi_->get_Baselines(&p);
    return Baselines(p);
}

long
Trace::stripChartMode() const
{
	long result;
    pi_->get_StripChartMode(&result);
	return result;
}

void
Trace::stripChartMode(long newValue)
{
	pi_->put_StripChartMode( newValue );
}

void
Trace::initializeStripChart(long nPoints, double xStart, double xEnd)
{
	pi_->InitializeStripChart( nPoints, xStart, xEnd );
}

void
Trace::addStripChartPoint(double x, double y, short ColorIndex)
{
	pi_->AddStripChartPoint(x, y, ColorIndex );
}

short
Trace::lineWidth() const
{
	short result;
	pi_->get_LineWidth( &result );
	return result;
}

void
Trace::lineWidth(short newValue)
{
    pi_->put_LineWidth( newValue );
}

long
Trace::userData() const
{
	long result;
	pi_->get_UserData(&result);
	return result;
}

void
Trace::userData(long newValue)
{
	pi_->put_UserData( newValue );
}

void
Trace::setColorIndicesDirect(long nPts, short * pIndices)
{
	pi_->SetColorIndicesDirect( nPts, pIndices );	
}

FilledRanges 
Trace::filledRanges() const
{
	CComPtr<ISADPFilledRanges> p;
    pi_->get_FilledRanges(&p);
    return FilledRanges(p);
}

long
Trace::orientation() const
{
	Orientation result;
	pi_->get_Orientation(&result);
	
	return result;
}

void
Trace::orientation(long newValue)
{
	pi_->put_Orientation( static_cast<Orientation>(newValue) );
}

double
Trace::autoAnnotationThreshold() const
{
	double result;
    pi_->get_AutoAnnotationThreshold( &result );
	return result;
}

void
Trace::autoAnnotationThreshold(double newValue)
{
	pi_->put_AutoAnnotationThreshold( newValue );
}

/*
void
Trace::setXY(VARIANT& XArray, VARIANT& YArray)
{
	
}
*/

void
Trace::setXYDirect(long nPts, double * pX, double * pY)
{
	pi_->SetXYDirect( nPts, pX, pY );
}

void
Trace::setXYFloatDirect(long nPts, double * pX, float * pY)
{
	pi_->SetXYFloatDirect( nPts, pX, pY );	
}

void
Trace::setXYPointers(long nPts, double * pX, double * pY)
{
	pi_->SetXYPointers( nPts, pX, pY );	
}

void
Trace::setXYFloatPointers(long nPts, double * pX, float * pY)
{
	pi_->SetXYFloatPointers( nPts, pX, pY );	
}

/*
void
Trace::SetXYChromSpec(LPDISPATCH piSpecOrChrom)
{
	static BYTE parms[] = VTS_DISPATCH ;
	
}
*/

void
Trace::clone( Trace& t )
{
   pi_->Clone( t.pi_ );
}
