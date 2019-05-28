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

#ifndef MSTOLERANCEFORM_HPP
#define MSTOLERANCEFORM_HPP

#include "adwidgets_global.hpp"
#include <adcontrols/quanresponsemethod.hpp>
#include <QWidget>

namespace adcontrols { class TargetingMethod;  class QuanResponseMethod; }

namespace adwidgets {
namespace Ui {
    class MSToleranceForm;
}

    class ADWIDGETSSHARED_EXPORT MSToleranceForm : public QWidget
    {
        Q_OBJECT

    public:
        explicit MSToleranceForm(QWidget *parent = 0);
        ~MSToleranceForm();

        void setTitle( const QString& );
        bool isChecked() const;
        void setChecked( bool );

        std::string toJson( bool pritty = false ) const;
        void fromJson( const std::string& );

        bool setContents( const adcontrols::TargetingMethod& );

        bool setContents( const adcontrols::QuanResponseMethod& );
        bool getContents( adcontrols::QuanResponseMethod& ) const;

    private:
        Ui::MSToleranceForm *ui;
    };

}

#endif // MSTOLERANCEFORM_HPP
