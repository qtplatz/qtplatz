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

#ifndef MSPEAKTABLE_HPP
#define MSPEAKTABLE_HPP

#include "adwidgets_global.hpp"
#include "tableview.hpp"
#include <QItemDelegate>
#include <adplugin_manager/lifecycle.hpp>
#include <memory>
#if !defined Q_MOC_RUN
# include <boost/variant.hpp>
#endif

class QStandardItemModel;

namespace adcontrols { class MSPeakInfo; class MassSpectrum; class ChemicalFormula; class Targeting;
    class MSPeaks; class MSPeak; }

namespace adwidgets {

    namespace detail { struct dataMayChanged; }

    class ADWIDGETSSHARED_EXPORT MSPeakTable : public TableView
                                             , public adplugin::LifeCycle {
        Q_OBJECT
        Q_INTERFACES( adplugin::LifeCycle )

    public:
        virtual ~MSPeakTable();
        explicit MSPeakTable(QWidget *parent = 0);
        void onInitialUpdate();
        QStandardItemModel& model();

        // adplugin::LifeCycle
        void OnCreate( const adportable::Configuration& ) override;
        void OnInitialUpdate() override;
        void onUpdate( boost::any& ) override;
        void OnFinalClose() override;
        bool getContents( boost::any& ) const override;
        bool setContents( boost::any& ) override;
        void * query_interface_workaround( const char * ) override;

        // interface for InfiTOF
        enum GETPEAKOPTS { AllPeaks, AssignedPeaks, SelectedPeaks };
        bool getMSPeaks( adcontrols::MSPeaks&, GETPEAKOPTS opt = AllPeaks ) const;
        bool getMSPeak( adcontrols::MSPeak&, int row ) const;

        virtual int findColumn( const QString& name ) const;

    protected:
        // reimplement QTableView
        void currentChanged( const QModelIndex&, const QModelIndex& ) override;
        void keyPressEvent( QKeyEvent * event ) override;

    signals:
        void valueChanged();
        void currentChanged( int idx, int fcn );
        void formulaChanged( int idx, int fcn );
        void triggerLockMass( const QVector< QPair<int, int> >& );

    public slots:
        void handleCopyToClipboard();
        void handleZoomedOnSpectrum( const QRectF& );   // zoomer zoomed
        void hideRows();
        void showRows();                       

    private slots:
        void handleValueChanged( const QModelIndex& );
        void showContextMenu( const QPoint& );
        void handleCopyAssignedPeaks();

    private:
        class impl;
        impl * impl_;

        void setPeakInfo( const adcontrols::MSPeakInfo& );
		void setPeakInfo( const adcontrols::MassSpectrum& );
        void setPeakInfo( const adcontrols::Targeting& );
		void setData( const adcontrols::MassSpectrum& );
        void formulaChanged( const QModelIndex& );
        void descriptionChanged( const QModelIndex& );
        static double exactMass( std::string );
        friend struct detail::dataMayChanged;
    };

    //////////////////////

    class MSPeakTableDelegate : public QItemDelegate {
        Q_OBJECT
    public:
        explicit MSPeakTableDelegate(QObject *parent = 0);
        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
		// void setEditorData(QWidget *editor, const QModelIndex &index) const override;
        void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
        // bool editorEvent( QEvent * event, QAbstractItemModel *
        //                   , const QStyleOptionViewItem&, const QModelIndex& ) override;
    signals:
        void valueChanged( const QModelIndex& ) const;

    public slots:
        
    };
}

#endif // MSPEAKTABLE_HPP
