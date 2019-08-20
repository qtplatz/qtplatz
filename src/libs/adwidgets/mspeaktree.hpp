/**************************************************************************
** Copyright (C) 2019 Toshinobu Hondo, Ph.D.
** Copyright (C) 2019 MS-Cheminformatics LLC, Toin, Mie Japan
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

#pragma once

#include "adwidgets_global.hpp"
#include "tableview.hpp"
#include "lifecycle.hpp"
#include <QTreeView>
#include <QItemDelegate>
#include <memory>
#include <functional>

class QStandardItemModel;
class QPrinter;

namespace adcontrols { class MSPeakInfo; class MassSpectrum; class ChemicalFormula; class Targeting;
    class MSPeaks; class MSPeak; }

namespace adwidgets {

    namespace detail { struct dataMayChanged; }

    class ADWIDGETSSHARED_EXPORT MSPeakTree : public QTreeView
                                            , public adplugin::LifeCycle {
        Q_OBJECT
        Q_INTERFACES( adplugin::LifeCycle )

    public:
        virtual ~MSPeakTree();
        explicit MSPeakTree(QWidget *parent = 0);
        void onInitialUpdate();
        QStandardItemModel& model();

        // adplugin::LifeCycle
        void OnCreate( const adportable::Configuration& ) override;
        void OnInitialUpdate() override;
        void onUpdate( boost::any&& ) override;
        void OnFinalClose() override;
        bool getContents( boost::any& ) const override;
        bool setContents( boost::any&& ) override;

        void * query_interface_workaround( const char * ) override;

        void dataMayChanged();

        enum Events { formula_changed, lockmass_triggered };
        typedef void( callback_t )( int event, const QVector< QPair<int, int> >& );

		void setContents( std::shared_ptr< adcontrols::MassSpectrum >, std::function< callback_t > callback );

        // interface for InfiTOF
        enum GETPEAKOPTS { AllPeaks, AssignedPeaks, SelectedPeaks };
        bool getMSPeaks( adcontrols::MSPeaks&, GETPEAKOPTS opt = AllPeaks ) const;
        bool getMSPeak( adcontrols::MSPeak&, int row ) const;

        // Targeting Result
        void setContents( std::pair< std::shared_ptr< adcontrols::MassSpectrum >, std::shared_ptr< const adcontrols::Targeting > >&& );

        virtual int findColumn( const QString& name ) const;

        virtual void addContextMenu( QMenu&, const QPoint&, std::shared_ptr< const adcontrols::MassSpectrum > ) const;

    protected:
        // reimplement QTableView
        void currentChanged( const QModelIndex&, const QModelIndex& ) override;
        void keyPressEvent( QKeyEvent * event ) override;

    signals:
        void valueChanged();
        void currentChanged( int idx, int fcn );
        void formulaChanged( int idx, int fcn );
        void triggerLockMass( const QVector< QPair<int, int> >& );
        void estimateScanLaw( const QVector< QPair<int, int> >& );
        void rescaleY( int protocol );

    public slots:
        void handleCopyToClipboard();
        void handleCopyAllToClipboard();
        void handleZoomedOnSpectrum( const QRectF&, int axis );   // zoomer zoomed
        void handlePrint( QPrinter&, QPainter& );

    private slots:
        void handleValueChanged( const QModelIndex& );
        void showContextMenu( const QPoint& );

    private:
        class impl;
        impl * impl_;

        void setPeakInfo( const adcontrols::Targeting&, std::shared_ptr< const adcontrols::MassSpectrum > );

		void setData( const adcontrols::MassSpectrum& );
        void updateData( const adcontrols::MassSpectrum& );
        void formulaChanged( const QModelIndex& );
        void descriptionChanged( const QModelIndex& );
        static double exactMass( std::string );
        //friend struct detail::dataMayChanged;
    };

    //////////////////////
}
