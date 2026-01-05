// This is a -*- C++ -*- header.
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

#pragma once

#include <QWidget>
#include <memory>
#include <tuple>

namespace portfolio { class Folium; }

namespace adcontrols {
    class MassSpectrum;
    class ProcessMethod;
    enum hor_axis: unsigned int;
}

namespace adprocessor {
    class dataprocessor;
}

namespace adplot { class SpectrumWidget; class PeakMarker; }

namespace lipidid {

    class MSSpectraWnd : public QWidget {
        Q_OBJECT
    public:
        explicit MSSpectraWnd( QWidget *parent = 0);
        ~MSSpectraWnd();

        void onPageSelected();
        void onDataChanged( const QString& foliumGuid, const QString& attGuid, int idx, int fcn );

    public slots:
        void handleSessionAdded( adprocessor::dataprocessor* );
        void handleProcessed( adprocessor::dataprocessor*, portfolio::Folium& );
        void handleSelectionChanged( adprocessor::dataprocessor*, portfolio::Folium& );
        void handleApplyMethod( const adcontrols::ProcessMethod& );
        void handlePrintCurrentView( const QString& outpdf );
        void handleAxisChanged( adcontrols::hor_axis );
        void handleCheckStateChanged( adprocessor::dataprocessor *, portfolio::Folium&, bool isChecked );

        void handleDataChanged( const portfolio::Folium& );
        void handleIdCompleted();
        void handleFormulaSelection( const QString& formula, double );
        void handleMatchedSelection();
        void handleSessionRemoved( const QString& file );

    signals:
        void zoomed( size_t, const QRectF& );

    private:
        void init();
        void handleDataChanged( const QString& dataGuid, int idx, int fcn, int column, const QVariant& );
        void handleCurrentChanged( const QString& dataGuid, int idx, int fcn );
        void handleSelected( const QRectF&, adplot::SpectrumWidget * );
        void redraw();

        class impl;
        impl * impl_;

    };
}
