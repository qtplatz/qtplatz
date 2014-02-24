/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/

#include "centroidmethodmodel.hpp"
#include <QDebug>

using namespace qtwidgets;

CentroidMethodModel::CentroidMethodModel( QObject * parent ) : QObject( parent )
{
}

CentroidMethodModel::ScanType
CentroidMethodModel::scanType() const
{
    ScanType t = static_cast<ScanType>( method_.peakWidthMethod() );
    qDebug() << "scanType: " << t;
    return t;
}

void
CentroidMethodModel::scanType( const ScanType t )
{
    qDebug() << "scanType(" << t << ")";
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
