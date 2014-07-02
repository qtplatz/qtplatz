/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#ifndef MSPEAKSWND_HPP
#define MSPEAKSWND_HPP

#include <QWidget>
#include <memory>

namespace adwplot { class Dataplot; }
namespace portfolio { class Folium; }
namespace adcontrols { class ProcessMethod; class MSPeaks; }

class QwtPlotMarker;
class QwtPlotCurve;

namespace dataproc {

    class Dataprocessor;

    class MSPeaksWnd : public QWidget  {
        Q_OBJECT
    public:
        explicit MSPeaksWnd(QWidget *parent = 0);

    signals:

    public slots:
        void handleSessionAdded( Dataprocessor* ) {}
        void handleProcessed( Dataprocessor*, portfolio::Folium& ) {}
        void handleSelectionChanged( Dataprocessor*, portfolio::Folium& ) {}
        void handleApplyMethod( const adcontrols::ProcessMethod& ) {}
        void handleCheckStateChanged( Dataprocessor*, portfolio::Folium&, bool ) {}
        void handlePrintCurrentView( const QString& outpdf );

        void handleSetData( int, const adcontrols::MSPeaks& );
        void handleSetData( const QString&, const adcontrols::MSPeaks& );

    private:
        std::vector< std::shared_ptr< adwplot::Dataplot > > plots_;
        std::vector< std::vector< std::shared_ptr< QwtPlotMarker > > > plotMarkers_;
        std::vector< std::vector< std::shared_ptr< QwtPlotCurve > > > plotCurves_;

        void init();
    };

}

#endif // MSPEAKSWND_HPP
