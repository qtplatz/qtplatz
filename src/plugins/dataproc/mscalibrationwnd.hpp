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

#ifndef MSCALIBRATIONWND_H
#define MSCALIBRATIONWND_H

#include <QWidget>
#include <QPrinter>
#include <memory>

namespace adcontrols {
    class MassSpectrum;
    class MSCalibrateResult;
	class MSCalibration;
    class ProcessMethod;
	class MSAssignedMasses;
	class MSProperty;
    class MSPeaks;
    enum hor_axis;
}

namespace adportable {  class Configuration; }
namespace portfolio  { class Folium; }

namespace dataproc {

    class Dataprocessor;
    class MSCalibrationWndImpl;

    class MSCalibrationWnd : public QWidget {
        Q_OBJECT
        public:
        // explicit MSCalibrationWnd(QWidget *parent = 0);
        MSCalibrationWnd( QWidget * parent = 0 );
      
    signals:
        void onSetData( const adcontrols::MSCalibrateResult&, const adcontrols::MassSpectrum& );
        void onPrint( QPrinter&, QPainter& );
      
    public slots:
        void handleSessionAdded( Dataprocessor* );
        void handleProcessed( Dataprocessor*, portfolio::Folium& );
        void handleSelectionChanged( Dataprocessor*, portfolio::Folium& );
        void handleApplyMethod( const adcontrols::ProcessMethod& );
        void handlePrintCurrentView( const QString& outpdf );
        void handleAxisChanged( adcontrols::hor_axis );

    private slots:
        void handleSelSummary( size_t idx, size_t fcn );
        void handleValueChanged();
        void handle_reassign_mass_requested();
        void handle_recalibration_requested();
        void handle_apply_calibration_to_dataset();
        void handle_apply_calibration_to_default();
        //---
        void handle_add_selection_to_peak_table( const adcontrols::MSPeaks& );
        
    private:
        std::shared_ptr<MSCalibrationWndImpl> pImpl_;
        bool readCalibSummary( adcontrols::MSAssignedMasses& );
        bool calibPolynomialFit( adcontrols::MSCalibrateResult&, const adcontrols::MSProperty& );
        void init();
    };

}

#endif // MSCALIBRATIONWND_H
