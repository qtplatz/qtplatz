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

#ifndef MSQUANTABLE_HPP
#define MSQUANTABLE_HPP

#pragma once

#include "adwidgets_global.hpp"
#include "tableview.hpp"
#include <memory>
#include <adwidgets/lifecycle.hpp>

class QItemDelegate;
class QStandardItemModel;
class QModelIndex;

namespace adcontrols { class MSQPeaks; class MSQPeak; }

namespace adwidgets {

    class ADWIDGETSSHARED_EXPORT MSQuanTable : public TableView
                                             , public adplugin::LifeCycle {

        Q_OBJECT
        Q_INTERFACES( adplugin::LifeCycle )

    public:
        explicit MSQuanTable( QWidget *parent = 0 );

        enum {
            column_component   // this is the primary id of component. if same key is assined to more than two formula, quant. result will be summed
            , column_formula      //
            , column_description
            , column_other
        };

        void setData( adcontrols::MSQPeaks * );
        void handleSelected( const QRectF&, bool isTime = false );

        // adplugin::LifeCycle
        void OnCreate( const adportable::Configuration& ) override;
        void OnInitialUpdate() override;
        void onUpdate( boost::any&& ) override;
        void OnFinalClose() override;
        bool getContents( boost::any& ) const override;
        bool setContents( boost::any&& ) override;
        void * query_interface_workaround( const char * ) override;

    protected:
        // reimplement QTableView
        void currentChanged( const QModelIndex&, const QModelIndex& ) override;
        void keyPressEvent( QKeyEvent * event ) override;

        void valueChanged();
        void currentChanged( int idx, int fcn );
        void formulaChanged( int idx, int fcn );

    public: // slots:
        void handleCopyToClipboard() override;
        void handle_zoomed( const QRectF& );   // zoomer zoomed

    private:
        bool inProgress_;
        QStandardItemModel * model_;
        void handleValueChanged( const QModelIndex& );
        void handleContextMenuRequested( const QPoint& );
        std::weak_ptr< adcontrols::MSQPeaks > qpks_;

        int find_row( const adcontrols::MSQPeak& );
        static void set_a_row( QStandardItemModel&, int row, const adcontrols::MSQPeak& );
        static void update_row( QStandardItemModel&, int row, const adcontrols::MSQPeak& );

    signals:
        void currentChanged( const QModelIndex& );
        void currentChanged( const QString& dataGuid, int idx, int fcn );
        void dataChanged( const QString& dataGuid, int idx, int fcn, int column, const QVariant& );
    };

}

#endif // MSQUANTABLE_HPP
