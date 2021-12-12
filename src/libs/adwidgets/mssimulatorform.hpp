/**************************************************************************
** Copyright (C) 2010-2021 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2021 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef MSSIMULATORFORM_HPP
#define MSSIMULATORFORM_HPP

#include <QWidget>
#include <memory>

namespace adcontrols { class MSSimulatorMethod; class MassSpectrometer; class MassSpectrum; }

namespace adwidgets {

    namespace Ui {
        class MSSimulatorForm;
    }

    class MSSimulatorForm : public QWidget {

        Q_OBJECT

    public:
        explicit MSSimulatorForm(QWidget *parent = 0);
        ~MSSimulatorForm();

        void OnInitialUpdate();
        void OnFinalClose();
        bool getContents( adcontrols::MSSimulatorMethod& ) const;
        bool setContents( const adcontrols::MSSimulatorMethod& );

        void setMassSpectrometer( std::shared_ptr< const adcontrols::MassSpectrometer > p );
        void setMassSpectrum( std::shared_ptr< const adcontrols::MassSpectrum > p );

    signals:
        void onValueChanged();
        void onLapChanged( int );
        void triggerProcess();

    private:
        Ui::MSSimulatorForm *ui;
        std::weak_ptr< const adcontrols::MassSpectrometer > massSpectrometer_;
        std::weak_ptr< const adcontrols::MassSpectrum > massSpectrum_;
    };

}

#endif // MSSIMULATORFORM_HPP
