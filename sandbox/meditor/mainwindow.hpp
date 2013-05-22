// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include <vector>
#include <adcontrols/centroidmethod.hpp>

class Plot;
class QwtPlotZoomer;
class QwtPlotPicker;
class QwtPlotPanner;

class CentroidMethodModel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

    Plot * plot_;
    QwtPlotZoomer * zoomer_;
    QwtPlotPicker * picker_;
    QwtPlotPanner * panner_;
    std::vector< double > x_;
    std::vector< double > y0_;
    std::vector< double > y1_;
    std::vector< double > y2_;
    std::vector< double > y3_;

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    adcontrols::CentroidMethod * pMethod_;
    CentroidMethodModel * pModel_;

    static double to_time( size_t );
    void draw_spectrum();
};

#endif // MAINWINDOW_H
