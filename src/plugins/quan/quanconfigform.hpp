/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef QUANCONFIGFORM_HPP
#define QUANCONFIGFORM_HPP

#include <QWidget>
#include <memory>

class QSpinBox;
namespace adcontrols { class QuanMethod; }

namespace quan {

    namespace Ui {
        class QuanConfigForm;
    }

    class QuanConfigForm : public QWidget {
        Q_OBJECT

    public:
        explicit QuanConfigForm( QWidget *parent = 0 );
        ~QuanConfigForm();
        bool setContents( const adcontrols::QuanMethod& );
        bool getContents( adcontrols::QuanMethod& );

        QSpinBox * spinLevels();
        QSpinBox * spinReplicates();

    signals:
        void onSampleInletChanged( int /* adcontrols::QuanSample::QuanInlet */);

    private slots:
        void on_pushButton_clicked();
        void on_radioButton_clicked();
        void on_radioButton_2_clicked();

    private:
        Ui::QuanConfigForm *ui;
    };

}
#endif // QUANCONFIGFORM_HPP
