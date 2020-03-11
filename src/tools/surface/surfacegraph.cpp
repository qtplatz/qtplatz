/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Data Visualization module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "surfacegraph.h"
#include <adportable/debug.hpp>
#include <QtDataVisualization/QValue3DAxis>
#include <QtDataVisualization/Q3DTheme>
#include <QtGui/QImage>
#include <QtCore/qmath.h>
#include <algorithm>

using namespace QtDataVisualization;

const int sampleCountX = 50;
const int sampleCountZ = 50;
const int heightMapGridStepX = 6;
const int heightMapGridStepZ = 6;
const float sampleMin = -8.0f;
const float sampleMax = 8.0f;

SurfaceGraph::SurfaceGraph(Q3DSurface *surface) : m_graph(surface)
{
    m_graph->setAxisX(new QValue3DAxis);
    m_graph->setAxisY(new QValue3DAxis);
    m_graph->setAxisZ(new QValue3DAxis);

    //! [0]
    m_sqrtSinProxy = new QSurfaceDataProxy();
    m_sqrtSinSeries = new QSurface3DSeries(m_sqrtSinProxy);
    //! [0]
    fillSqrtSinProxy();

    //////////////////////////
    sfProxy_ = new QSurfaceDataProxy();
    sfSeries_ = new QSurface3DSeries( sfProxy_ );
    //////////////////////////

    // //! [2]
    // QImage heightMapImage(":/maps/mountain");
    // m_heightMapProxy = new QHeightMapSurfaceDataProxy(heightMapImage);
    // m_heightMapSeries = new QSurface3DSeries(m_heightMapProxy);
    // m_heightMapSeries->setItemLabelFormat(QStringLiteral("(@xLabel, @zLabel): @yLabel"));
    // m_heightMapProxy->setValueRanges(34.0f, 40.0f, 18.0f, 24.0f);
    // //! [2]
    // m_heightMapWidth = heightMapImage.width();
    // m_heightMapHeight = heightMapImage.height();
}

SurfaceGraph::~SurfaceGraph()
{
    delete m_graph;
}

//! [1]
void SurfaceGraph::fillSqrtSinProxy()
{
    float stepX = (sampleMax - sampleMin) / float(sampleCountX - 1);
    float stepZ = (sampleMax - sampleMin) / float(sampleCountZ - 1);

    QSurfaceDataArray *dataArray = new QSurfaceDataArray;
    dataArray->reserve(sampleCountZ);
    for (int i = 0 ; i < sampleCountZ ; i++) {
        QSurfaceDataRow *newRow = new QSurfaceDataRow(sampleCountX);
        // Keep values within range bounds, since just adding step can cause minor drift due
        // to the rounding errors.
        float z = qMin(sampleMax, (i * stepZ + sampleMin));
        int index = 0;
        for (int j = 0; j < sampleCountX; j++) {
            float x = qMin(sampleMax, (j * stepX + sampleMin));
            float R = qSqrt(z * z + x * x) + 0.01f;
            float y = (qSin(R) / R + 0.24f) * 1.61f;
            (*newRow)[index++].setPosition(QVector3D(x, y, z));
        }
        *dataArray << newRow;
    }

    m_sqrtSinProxy->resetArray(dataArray);
}
//! [1]
void
SurfaceGraph::enableSurface( const adcontrols::Surface& surface )
{
    auto matrix = surface.getSurface();

    // float stepX = (sampleMax - sampleMin) / float(sampleCountX - 1);
    // float stepZ = (sampleMax - sampleMin) / float(sampleCountZ - 1);

    QSurfaceDataArray * dataArray = new QSurfaceDataArray;
    // dataArray->reserve(sampleCountZ);

    std::pair< float, float > minmax{ matrix(0,0), matrix(0,0) };

    size_t size1 = matrix.size1();
    size_t size2 = matrix.size2() < 100 ? matrix.size2() : 100;

    for ( size_t i = 0; i < size1; ++i ) {      // elapsed time; a.k.a. retention time
        QSurfaceDataRow * row = new QSurfaceDataRow( matrix.size2() );

        for ( size_t j = 0; j < size2; ++j ) {  // spectrum
            float x = float( j );
            float y = float( matrix(i,j) );
            (*row) << QVector3D( x, y, float( i ) );
            minmax.first = std::min( minmax.first, y );
            minmax.second = std::max( minmax.second, y );
        }
        ADDEBUG() << i << ", z=" << matrix( i, 10 );
        *dataArray << row;
    }

    sfProxy_->resetArray(dataArray);

    sfSeries_->setDrawMode(QSurface3DSeries::DrawSurfaceAndWireframe);
    sfSeries_->setFlatShadingEnabled(true);
    
    m_graph->axisX()->setLabelFormat("%.2f");
    m_graph->axisZ()->setLabelFormat("%.2f");
    
    m_graph->axisX()->setRange( 0, float( size1 ) );
    m_graph->axisY()->setRange( -0.1, 0.5 ); // minmax.first, minmax.second ); // 0.0f, 2.0f);
    m_graph->axisZ()->setRange( 0, float( size2 ) ); 

    m_graph->axisX()->setLabelAutoRotation(30);
    m_graph->axisY()->setLabelAutoRotation(90);
    m_graph->axisZ()->setLabelAutoRotation(60);

    for ( auto series: m_graph->seriesList() )
        m_graph->removeSeries( series );

    m_graph->addSeries( sfSeries_ );

    // Reset range sliders for Sqrt&Sin
    // m_rangeMinX = 0; //sampleMin;
    // m_rangeMinZ = minmax.first;
    
    // m_stepX = 1; // (sampleMax - sampleMin) / float(sampleCountX - 1);
    // m_stepZ = 1; // (sampleMax - sampleMin) / float(sampleCountZ - 1);

    // m_axisMinSliderX->setMaximum( size1 - 2);
    // m_axisMinSliderX->setValue(0);
    
    // m_axisMaxSliderX->setMaximum( size1 - 1);
    // m_axisMaxSliderX->setValue( size1 - 1 );
    
    // m_axisMinSliderZ->setMaximum( minmax.second );
    // m_axisMinSliderZ->setValue(0);
    
    // m_axisMaxSliderZ->setMaximum( minmax.second );
    // m_axisMaxSliderZ->setValue( minmax.second );
}

void SurfaceGraph::enableSqrtSinModel(bool enable)
{
    if (enable) {
        //! [3]
        m_sqrtSinSeries->setDrawMode(QSurface3DSeries::DrawSurfaceAndWireframe);
        m_sqrtSinSeries->setFlatShadingEnabled(true);

        m_graph->axisX()->setLabelFormat("%.2f");
        m_graph->axisZ()->setLabelFormat("%.2f");
        m_graph->axisX()->setRange(sampleMin, sampleMax);
        m_graph->axisY()->setRange(0.0f, 2.0f);
        m_graph->axisZ()->setRange(sampleMin, sampleMax);
        m_graph->axisX()->setLabelAutoRotation(30);
        m_graph->axisY()->setLabelAutoRotation(90);
        m_graph->axisZ()->setLabelAutoRotation(30);

        //m_graph->removeSeries(m_heightMapSeries);
        m_graph->addSeries(m_sqrtSinSeries);
        //! [3]

        //! [8]
        // Reset range sliders for Sqrt&Sin
        m_rangeMinX = sampleMin;
        m_rangeMinZ = sampleMin;
        m_stepX = (sampleMax - sampleMin) / float(sampleCountX - 1);
        m_stepZ = (sampleMax - sampleMin) / float(sampleCountZ - 1);
        m_axisMinSliderX->setMaximum(sampleCountX - 2);
        m_axisMinSliderX->setValue(0);
        m_axisMaxSliderX->setMaximum(sampleCountX - 1);
        m_axisMaxSliderX->setValue(sampleCountX - 1);
        m_axisMinSliderZ->setMaximum(sampleCountZ - 2);
        m_axisMinSliderZ->setValue(0);
        m_axisMaxSliderZ->setMaximum(sampleCountZ - 1);
        m_axisMaxSliderZ->setValue(sampleCountZ - 1);
        //! [8]
    }
}

void SurfaceGraph::adjustXMin(int min)
{
    float minX = m_stepX * float(min) + m_rangeMinX;

    int max = m_axisMaxSliderX->value();
    if (min >= max) {
        max = min + 1;
        m_axisMaxSliderX->setValue(max);
    }
    float maxX = m_stepX * max + m_rangeMinX;

    setAxisXRange(minX, maxX);
}

void SurfaceGraph::adjustXMax(int max)
{
    float maxX = m_stepX * float(max) + m_rangeMinX;

    int min = m_axisMinSliderX->value();
    if (max <= min) {
        min = max - 1;
        m_axisMinSliderX->setValue(min);
    }
    float minX = m_stepX * min + m_rangeMinX;

    setAxisXRange(minX, maxX);
}

void SurfaceGraph::adjustZMin(int min)
{
    float minZ = m_stepZ * float(min) + m_rangeMinZ;

    int max = m_axisMaxSliderZ->value();
    if (min >= max) {
        max = min + 1;
        m_axisMaxSliderZ->setValue(max);
    }
    float maxZ = m_stepZ * max + m_rangeMinZ;

    setAxisZRange(minZ, maxZ);
}

void SurfaceGraph::adjustZMax(int max)
{
    float maxX = m_stepZ * float(max) + m_rangeMinZ;

    int min = m_axisMinSliderZ->value();
    if (max <= min) {
        min = max - 1;
        m_axisMinSliderZ->setValue(min);
    }
    float minX = m_stepZ * min + m_rangeMinZ;

    setAxisZRange(minX, maxX);
}

//! [5]
void SurfaceGraph::setAxisXRange(float min, float max)
{
    m_graph->axisX()->setRange(min, max);
}

void SurfaceGraph::setAxisZRange(float min, float max)
{
    m_graph->axisZ()->setRange(min, max);
}
//! [5]

//! [6]
void SurfaceGraph::changeTheme(int theme)
{
    m_graph->activeTheme()->setType(Q3DTheme::Theme(theme));
}
//! [6]

void SurfaceGraph::setBlackToYellowGradient()
{
    //! [7]
    QLinearGradient gr;
    gr.setColorAt(0.0, Qt::black);
    gr.setColorAt(0.33, Qt::blue);
    gr.setColorAt(0.67, Qt::red);
    gr.setColorAt(1.0, Qt::yellow);

    m_graph->seriesList().at(0)->setBaseGradient(gr);
    m_graph->seriesList().at(0)->setColorStyle(Q3DTheme::ColorStyleRangeGradient);
    //! [7]
}

void SurfaceGraph::setGreenToRedGradient()
{
    QLinearGradient gr;
    gr.setColorAt(0.0, Qt::darkGreen);
    gr.setColorAt(0.5, Qt::yellow);
    gr.setColorAt(0.8, Qt::red);
    gr.setColorAt(1.0, Qt::darkRed);

    m_graph->seriesList().at(0)->setBaseGradient(gr);
    m_graph->seriesList().at(0)->setColorStyle(Q3DTheme::ColorStyleRangeGradient);
}

