// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2022-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2022-2022 MS-Cheminformatics LLC
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
#include <QWidget>
#include <QStandardItemModel>
//#include <adplugin_manager/lifecycle.hpp>
#include <adwidgets/lifecycle.hpp>
#include <adportable/configuration.hpp>
#include <memory>

namespace adcontrols {
    class CentroidMethod;
    class datafile;
    class ProcessMethod;
}

namespace Ui {
    class CentroidForm;
}

namespace adwidgets {

    class ADWIDGETSSHARED_EXPORT CentroidWidget : public QWidget
                                                , public adplugin::LifeCycle {
        Q_OBJECT
        Q_INTERFACES( adplugin::LifeCycle )
    public:
        explicit CentroidWidget(QWidget *parent = 0);
        virtual ~CentroidWidget();

        // adplugin::LifeCycle
        void OnCreate( const adportable::Configuration& ) override;
        void OnInitialUpdate() override;
        void OnFinalClose() override;
        bool getContents( boost::any& ) const override;
        bool setContents( boost::any&& ) override;

        // QWidget
        QSize sizeHint() const override;
        //<----
        adcontrols::CentroidMethod getValue() const;
        void setValue( const adcontrols::CentroidMethod & );

    private:
        class impl;
        std::unique_ptr< impl > impl_;
    private:
        // void update_data();
        // void update_data( const adcontrols::CentroidMethod& );

    public slots:
        void getContents( adcontrols::ProcessMethod& );
		virtual void update();

    signals:
        void apply( adcontrols::ProcessMethod& );
        void valueChanged();
        void triggerProcess( const QString& );

    private slots:
        void handleAnalyzerTypeChanged( int );
        void handlePeakWidthChanged( double );

        void on_doubleSpinBox_peakwidth_valueChanged(double arg1);
        void on_doubleSpinBox_centroidfraction_valueChanged(double arg1);
        void on_noiseFilterMethod_stateChanged(int arg1);
        void on_cutoffMHz_valueChanged(int arg1);
    };

}
