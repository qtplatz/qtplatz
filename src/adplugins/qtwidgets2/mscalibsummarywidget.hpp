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
#include <adplugin/lifecycle.hpp>
#include <memory>

namespace adcontrols {
    class MassSpectrum;
    class MSReferences;
    class MSCalibrateResult;
    class MSAssignedMasses;
}

class QStandardItemModel;

namespace qtwidgets2 {

    class MSCalibSummaryDelegate;

    class MSCalibSummaryWidget : public QTableView, public adplugin::LifeCycle { 
        Q_OBJECT
    public:
        ~MSCalibSummaryWidget();
        explicit MSCalibSummaryWidget(QWidget *parent = 0);

        // adplugin::LifeCycle
        virtual void OnCreate( const adportable::Configuration& );
        virtual void OnInitialUpdate();
        virtual void OnUpdate( boost::any& );
        virtual void OnFinalClose();
        bool getContents( boost::any& ) const;
        bool setContents( boost::any& );

        // <--
    protected:
        // reimplement QTableView
        virtual void currentChanged( const QModelIndex&, const QModelIndex& );
        virtual void keyPressEvent( QKeyEvent * event );

    signals:
        void valueChanged();
        void currentChanged( size_t row );
        void applyTriggered();
        void applyPeakAssign();

    public slots:
        void setData( const adcontrols::MSCalibrateResult&, const adcontrols::MassSpectrum& );
        void getLifeCycle( adplugin::LifeCycle*& );
        void showContextMenu( const QPoint& );

   private slots:
        void handleEraseFormula();
        void handleUpdateCalibration();
        void handleCopyToClipboard();
        void handleClearFormulae();
        void handleValueChanged( const QModelIndex& );
        void handleUpdatePeakAssign();

    private:
        std::unique_ptr< QStandardItemModel > pModel_;
        std::unique_ptr< MSCalibSummaryDelegate > pDelegate_;
        std::unique_ptr< adcontrols::MSReferences > pReferences_;
        std::unique_ptr< adcontrols::MassSpectrum > pCalibrantSpectrum_;
        bool inProgress_;
        std::vector< size_t > indecies_;
        void getAssignedMasses( adcontrols::MSAssignedMasses& ) const;
    };

}


