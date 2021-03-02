/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef MSSIMULATORWIDGET_HPP
#define MSSIMULATORWIDGET_HPP

#include "adwidgets_global.hpp"

#include <adwidgets/lifecycle.hpp>
#include <QWidget>

class QMenu;

namespace adcontrols {
    class MassSpectrometer;
    class MassSpectrum;
    class MSSimulatorMethod;
}

namespace adwidgets {

    class ADWIDGETSSHARED_EXPORT MSSimulatorWidget : public QWidget
                                                   , public adplugin::LifeCycle {

        Q_OBJECT
        Q_INTERFACES( adplugin::LifeCycle )

    public:
        explicit MSSimulatorWidget(QWidget *parent = 0);
        ~MSSimulatorWidget();

        static QWidget * create( QWidget * parent );

        // adplugin::LifeCycle
        void OnCreate( const adportable::Configuration& ) override;
        void OnInitialUpdate() override;
        void onUpdate( boost::any&& ) override;
        void OnFinalClose() override;
        bool getContents( boost::any& ) const override;
        bool setContents( boost::any&& ) override;
        bool setContents( boost::any&&, const std::string& json ) override;
        //
        void setTimeSquaredScanLaw( double flength, double acceleratorVoltage, double tdelay );
        void setMassSpectrometer( std::shared_ptr< const adcontrols::MassSpectrometer > );

        std::shared_ptr< adcontrols::MassSpectrum > massSpectrum() const; // return simulated spectrum

    private:
        void handleContextMenu( QMenu&, const QPoint& );
        std::unique_ptr< adcontrols::MSSimulatorMethod > getMethod() const;

    signals:
        void triggerProcess( const QString& );

    public slots:

    private slots:
        void run();
        void handleLapChanged( int );
        void handleDataChanged( const QModelIndex&, const QModelIndex& );

    private:
        class impl;
        impl * impl_;
    };

}

#endif // MSSIMULATORWIDGET_HPP
