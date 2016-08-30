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

#pragma once

#include "adwidgets_global.hpp"
#include <adplugin_manager/lifecycle.hpp>
#include <QWidget>
#include <memory>

class QStandardItem;
class QStandardItemModel;

namespace adcontrols { class MassSpectrometer; class CountingMethod; }

namespace adwidgets {

    class MolTableView;

    class ADWIDGETSSHARED_EXPORT CountingWidget : public QWidget
                                                , public adplugin::LifeCycle {
        Q_OBJECT
        Q_INTERFACES( adplugin::LifeCycle )

    public:
        explicit CountingWidget(QWidget *parent = 0);
        ~CountingWidget();

        // adplugin::LifeCycle
        void OnCreate( const adportable::Configuration& ) override;
        void OnInitialUpdate() override;
        void onUpdate( boost::any&& ) override;
        void OnFinalClose() override;
        bool getContents( boost::any& ) const override;
        bool setContents( boost::any&& ) override;   
        //
        enum column_type {
            CountingEnable, CountingFormula
            , CountingRangeFirst, CountingRangeWidth
            , CountingProtocol, CountingLaps
        };
        
        void setMassSpectrometer( std::shared_ptr< const adcontrols::MassSpectrometer > );
        bool getContents( adcontrols::CountingMethod& ) const;
        bool setContents( const adcontrols::CountingMethod& );
        void setValue( int row, column_type column, const QVariant& value );
        QVariant value( int row, column_type column ) const;

    signals:
        void editChanged( int row, column_type column, const QVariant& value );
        void valueChanged( int row, column_type column, const QVariant& );
        void rowsRemoved( int, int );

    public slots:

    private slots:
        void addRow();
        void handleItemChanged( const QStandardItem * );

    private:
        void setup( MolTableView * );
        std::unique_ptr< QStandardItemModel > model_;
        std::weak_ptr< const adcontrols::MassSpectrometer > spectrometer_;
    };

}
