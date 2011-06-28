//
#include "centroidmethodmodel.hpp"
#include <QDebug>

CentroidMethodModel::CentroidMethodModel( QObject * parent ) : QObject( parent )
{
}

CentroidMethodModel::ScanType
CentroidMethodModel::scanType() const
{
    return static_cast<ScanType>( method_.peakWidthMethod() );
}

void
CentroidMethodModel::scanType( const ScanType t )
{
    method_.peakWidthMethod( static_cast< adcontrols::CentroidMethod::ePeakWidthMethod>(t) );
}

CentroidMethodModel::AreaHeight
CentroidMethodModel::areaHeight() const
{
    return method_.centroidAreaIntensity() ? Area : Height;
}

void
CentroidMethodModel::areaHeight( AreaHeight t )
{
    method_.centroidAreaIntensity( t == Area ? true : false );
}

double
CentroidMethodModel::baseline_width() const
{
    return method_.baselineWidth();
}

void
CentroidMethodModel::baseline_width( double v )
{
    return method_.baselineWidth( v );
}

double
CentroidMethodModel::peak_centroid_fraction() const
{
    return method_.peakCentroidFraction() * 100;
}

void
CentroidMethodModel::peak_centroid_fraction( double v )
{
    return method_.peakCentroidFraction( v / 100 );
}
////////////////////////////
