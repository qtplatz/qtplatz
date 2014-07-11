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

#ifndef MSCALIBRATEFORM_HPP
#define MSCALIBRATEFORM_HPP

#include <QWidget>

namespace Ui {
class MSCalibrateForm;
}

class QLabel;
class QSpinBox;
class QDoubleSpinBox;

namespace adcontrols { class MSCalibrateMethod; class MSReference; }

namespace adwidgets {

    class MSReferenceDialog;

    class MSCalibrateForm : public QWidget  {
        Q_OBJECT

    public:
        explicit MSCalibrateForm(QWidget *parent = 0);
        ~MSCalibrateForm();

        void getContents( adcontrols::MSCalibrateMethod& );
        void setContents( const adcontrols::MSCalibrateMethod& );

    private:
        Ui::MSCalibrateForm *ui;
        MSReferenceDialog * dlg_;

        void handleReferenceDlg();

    signals:
        void addReference( const adcontrols::MSReference& );

    };

}

#endif // MSCALIBRATEFORM_HPP
