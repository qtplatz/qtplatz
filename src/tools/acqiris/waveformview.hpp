/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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
**************************************************************************/

#pragma once

#include <qwt_plot.h>
#include <memory>

class XYSeriesData;
class QwtPlotCurve;

namespace acqrscontrols { namespace aqdrv4 { class waveform; } }

class WaveformView : public QwtPlot {

    Q_OBJECT

public:
    WaveformView( QWidget * parent = 0 );
    ~WaveformView();

    void setTitle( const QString& );
    void setFooter( const QString& );
    
    void setData( std::shared_ptr< const acqrscontrols::aqdrv4::waveform > );

private:
    std::unique_ptr< QwtPlotCurve > curve_;
};

