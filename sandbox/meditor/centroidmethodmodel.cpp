//
#include "centroidmethodmodel.hpp"
#include <QDebug>

CentroidMethodModel::CentroidMethodModel( QObject * parent ) : QObject( parent )
{
    //method_.peakWidthMethod( adcontrols::CentroidMethod::ePeakWidthConstant );
}

CentroidMethodModel::ScanType
CentroidMethodModel::scanType() const
{
    ScanType t = static_cast<ScanType>( method_.peakWidthMethod() );
    return t;
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

double
CentroidMethodModel::peakwidth_tof_in_da() const
{
    return method_.rsTofInDa();
}

void
CentroidMethodModel::peakwidth_tof_in_da( double t )
{
    method_.rsTofInDa( t );
}

double
CentroidMethodModel::peakwidth_tof_at_mz() const
{
   return method_.rsTofAtMz();
}

void
CentroidMethodModel::peakwidth_tof_at_mz( double t )
{
    method_.rsTofAtMz( t );
}

double
CentroidMethodModel::peakwidth_propo_in_ppm() const
{
    return method_.rsPropoInPpm();
}

void
CentroidMethodModel::peakwidth_propo_in_ppm( double t )
{
    method_.rsPropoInPpm( t );
}

double
CentroidMethodModel::peakwidth_const_in_da() const
{
    return method_.rsConstInDa();
}

void
CentroidMethodModel::peakwidth_const_in_da( double t )
{
    method_.rsConstInDa( t );
}

////////////////////////////
