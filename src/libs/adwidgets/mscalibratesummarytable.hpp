/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#ifndef MSCALIBRATESUMMARYTABLE_HPP
#define MSCALIBRATESUMMARYTABLE_HPP

#pragma once

#include "adwidgets_global.hpp"

#include <QTableView>
#include <adplugin/lifecycle.hpp>
#include <memory>

namespace adcontrols {
    class MassSpectrum;
    class MSReferences;
    class MSCalibrateResult;
    class MSAssignedMasses;
    class MSPeaks;
}

class QPrinter;
class QStandardItemModel;

namespace adwidgets {

    class ADWIDGETSSHARED_EXPORT MSCalibrateSummaryTable : public QTableView
                                                         , public adplugin::LifeCycle {

        Q_OBJECT

    public:
        ~MSCalibrateSummaryTable();
        explicit MSCalibrateSummaryTable(QWidget *parent = 0);

        // adplugin::LifeCycle
        void OnCreate( const adportable::Configuration& ) override;
        void OnInitialUpdate() override;
        void onUpdate( boost::any& ) override;
        void OnFinalClose() override;
        bool getContents( boost::any& ) const;
        bool setContents( boost::any& );

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
        void on_apply_calibration_to_all();
        void on_apply_calibration_to_default();
        void on_add_selection_to_peak_table( const adcontrols::MSPeaks& );

    public slots:
        void setData( const adcontrols::MSCalibrateResult&, const adcontrols::MassSpectrum& );
        void getLifeCycle( adplugin::LifeCycle*& );
        void showContextMenu( const QPoint& );
        void handle_zoomed( const QRectF& );   // zoomer zoomed
        void handle_selected( const QRectF& ); // picker selected
        void handlePrint( QPrinter&, QPainter& );
        void hideRows();
        void showRows();

   private slots:
        void handleEraseFormula();
        void handleCopyToClipboard();
        void handlePasteFromClipboard();
        void handleValueChanged( const QModelIndex& );

        void recalcPolynomials();
        void assignMassOnSpectrum();
        void applyCalibrationToDataset();
        void saveAsDefaultCalibration();
        void copySummaryToClipboard();
        void addSelectionToPeakTable();
        
    private:
        bool inProgress_;
        std::unique_ptr< QStandardItemModel > pModel_;
        std::unique_ptr< adcontrols::MSReferences > pReferences_;
        std::unique_ptr< adcontrols::MSCalibrateResult > pCalibResult_;
        std::unique_ptr< adcontrols::MassSpectrum > pCalibrantSpectrum_;

        void getAssignedMasses( adcontrols::MSAssignedMasses& ) const;
        bool modifyModelData( const std::vector< std::pair< int, int > >& );
        bool createModelData( const std::vector< std::pair< int, int > >& );
        bool setAssignedData( int row, int fcn, int idx, const adcontrols::MSAssignedMasses& );
        void setEditable( int row, bool enable = false );
        void formulaChanged( const QModelIndex& );
    };

}

#endif // MSCALIBRATESUMMARYTABLE_HPP
