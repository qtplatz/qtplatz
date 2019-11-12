/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

#pragma once

#include "constants.hpp"
#include <QWidget>

namespace aqmd3controls { class method; }

namespace aqmd3widgets {

    namespace Ui {
        class aqmd3Form;
    }

    class aqmd3Form : public QWidget {
        Q_OBJECT

    public:
        explicit aqmd3Form( QWidget *parent = 0 );
        ~aqmd3Form();

        void onInitialUpdate();

        void setContents( const aqmd3controls::method& );

        void getContents( aqmd3controls::method& );

        void onHandleValue( idCategory, int, const QVariant& );

        void setEnabled( const QString&, bool );

    signals:
        void valueChanged( idCategory, int channel, const QVariant& );

    private:
        Ui::aqmd3Form *ui;
        double sampRate_;  // Hz
    };

}
