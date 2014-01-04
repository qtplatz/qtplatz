// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
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

#pragma once

#include <QTableView>
#include <QPrinter>
#include <adplugin/lifecycle.hpp>
#include <memory>

namespace adcontrols {
    class MassSpectrum;
    class MSReferences;
    class MSCalibrateResult;
    class MSAssignedMasses;
    class MSPeaks;
}

class QStandardItemModel;

namespace qtwidgets2 {

    class MSCalibSummaryDelegate;

    class MSCalibSummaryWidget : public QTableView, public adplugin::LifeCycle { 
        Q_OBJECT
    public:
        ~MSCalibSummaryWidget();
        explicit MSCalibSummaryWidget(QWidget *parent = 0);

        enum {
            c_time
            , c_formula
            , c_exact_mass
            , c_mass
            , c_mass_error_mDa
            , c_mass_calibrated
            , c_mass_error_calibrated_mDa
            , c_is_enable
            , c_delta_mass // d-mass from currnet focused peak
            , c_intensity
            , c_mode // analyzer mode id, a.k.a. reflectron|linear, or number of turns on InfiTOF
            , c_fcn  // segment id
            , c_index // keep ms index for 'sort', invisible
            , c_time_normalized
            , c_number_of_columns
            , c_flags_ // -- not in use -- out of order
        };

        // adplugin::LifeCycle
        void OnCreate( const adportable::Configuration& ) override;
        void OnInitialUpdate() override;
        void onUpdate( boost::any& ) override;
        void OnFinalClose() override;
        bool getContents( boost::any& ) const;
        bool setContents( boost::any& );

        // <--
    protected:
        // reimplement QTableView
        void currentChanged( const QModelIndex&, const QModelIndex& ) override;
        void keyPressEvent( QKeyEvent * event ) override;

    signals:
        void valueChanged();
        void currentChanged( size_t idx, size_t fcn );
        void on_recalibration_requested();
        void on_reassign_mass_requested();
        void on_apply_calibration_to_dataset();
        void on_apply_calibration_to_default();
        void on_add_selection_to_peak_table( const adcontrols::MSPeaks& );

    public slots:
        void setData( const adcontrols::MSCalibrateResult&, const adcontrols::MassSpectrum& );
        void getLifeCycle( adplugin::LifeCycle*& );
        void showContextMenu( const QPoint& );
        void handle_zoomed( const QRectF& );   // zoomer zoomed
        void handle_selected( const QRectF& ); // picker selected
        void handlePrint( QPrinter&, QPainter& );

   private slots:
        void handleEraseFormula();

        void handleCopyToClipboard();
        void handlePasteFromClipboard();
        void handleValueChanged( const QModelIndex& );

    private:
        std::unique_ptr< QStandardItemModel > pModel_;
        std::unique_ptr< MSCalibSummaryDelegate > pDelegate_;
        std::unique_ptr< adcontrols::MSReferences > pReferences_;
        std::unique_ptr< adcontrols::MSCalibrateResult > pCalibResult_;
        std::unique_ptr< adcontrols::MassSpectrum > pCalibrantSpectrum_;
        bool inProgress_;
        // std::vector< std::pair< uint32_t, uint32_t > > indecies_; // fcn, idx
        void getAssignedMasses( adcontrols::MSAssignedMasses& ) const;
        bool modifyModelData( const std::vector< std::pair< int, int > >& );
        bool createModelData( const std::vector< std::pair< int, int > >& );
        bool setAssignedData( int row, int fcn, int idx, const adcontrols::MSAssignedMasses& );
        void setEditable( int row, bool enable = false );
        void formulaChanged( const QModelIndex& );
        void copySummaryToClipboard();
        void addSelectionToPeakTable();
    };

}


