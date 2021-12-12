/**************************************************************************
** Copyright (C) 2010-2021 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2021 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include <adwidgets/lifecycle.hpp>
#include <QWidget>
#include <boost/optional.hpp>
#include <memory>

class QMenu;

namespace adcontrols {
    class MassSpectrometer;
    class MassSpectrum;
    class MSSimulatorMethod;
    class moltable;
}

namespace accutof {

    class MoleculesWidget : public QWidget
                              , public adplugin::LifeCycle {

        Q_OBJECT
        Q_INTERFACES( adplugin::LifeCycle )

     public:
        explicit MoleculesWidget(QWidget *parent = 0);
        ~MoleculesWidget();

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
        void setMassSpectrometer( std::shared_ptr< const adcontrols::MassSpectrometer > );

        std::string readJson() const;
        static boost::optional< adcontrols::moltable > json_to_moltable( const std::string& json );

    signals:
        void valueChanged( const QString& json );

    private:
        // void handleContextMenu( QMenu&, const QPoint& );

    public slots:

    private slots:
        void handleDataChanged( const QModelIndex&, const QModelIndex&, const QVector<int>& );
        void handleRowsRemoved( const QModelIndex&, int, int );

    private:
        class impl;
        impl * impl_;
    };

}
