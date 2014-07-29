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

#ifndef MSLOCKFORM_HPP
#define MSLOCKFORM_HPP

#include "adwidgets_global.hpp"
#include <QWidget>

namespace adcontrols { class MSLockMethod; class ProcessMethod; }

namespace adwidgets {

    namespace Ui {
        class MSLockForm;
    }

    class ADWIDGETSSHARED_EXPORT MSLockForm : public QWidget
    {
        Q_OBJECT

    public:
        explicit MSLockForm( QWidget *parent = 0 );
        ~MSLockForm();

        void setTitle( const QString& );
        bool isChecked();
        void setChecked( bool );

        bool setContents( const adcontrols::MSLockMethod& );
        bool getContents( adcontrols::MSLockMethod& );

        bool setContents( const adcontrols::ProcessMethod&, bool forceCreate = false );
        bool getContents( adcontrols::ProcessMethod& );

    private:
        Ui::MSLockForm *ui;

    signals:
        void toggled( bool );
    };

}

#endif // MSLOCKFORM_HPP
