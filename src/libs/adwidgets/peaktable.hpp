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

#include "adwidgets_global.hpp"
#include "tableview.hpp"
#include <adwidgets/lifecycle.hpp>
#include <memory>

class QStandardItemModel;

namespace adcontrols {
    class Peaks;
    class Peak;
    class Baselines;
    class Chromatogram;
    class PeakResult;
}

namespace adwidgets {

    namespace peaktable {
        enum id_column {
            c_id
            , c_name
            , c_tr
            , c_area
            , c_height
            , c_width
            , c_ntp
            , c_rs
            , c_asymmetry
            , c_capacityfactor
            , c_cid
            , nbr_of_columns
        };
    }

    class ADWIDGETSSHARED_EXPORT PeakTable : public TableView
                    , public adplugin::LifeCycle {
        Q_OBJECT
        Q_INTERFACES( adplugin::LifeCycle )

     public:
        ~PeakTable();
        explicit PeakTable(QWidget *parent = 0);

        // adplugin::LifeCycle
        void OnCreate( const adportable::Configuration& ) override;
        void OnInitialUpdate() override;
        void OnFinalClose() override;
        bool getContents( boost::any& ) const override;
        bool setContents( boost::any&& ) override;
        int peakId( int row ) const;
        void addData( adcontrols::PeakResult&&, size_t idx, bool clearAll = false );

    signals:
        void valueChanged( int idx, int cid, const QModelIndex& );
        void currentChanged( int idx );
        void peaksAboutToBeRemoved( const std::vector< std::pair<int, int> >& rows );
    protected:
        void contextMenuEvent( QContextMenuEvent * event ) override;

    public slots:
		void setData( const adcontrols::PeakResult& );
        virtual void handleDeleteNoSelection();

    private slots:
        void setData( const adcontrols::Peaks&, bool isCounting );

    protected:

    private:
        void add( const adcontrols::Peak&, size_t idx );

        // QTableView
        void currentChanged( const QModelIndex&, const QModelIndex& ) override;

    private:
        QStandardItemModel * model_;
    };
}
