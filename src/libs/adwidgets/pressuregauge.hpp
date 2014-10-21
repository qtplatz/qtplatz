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

#ifndef PRESSUREGUAGE_HPP
#define PRESSUREGUAGE_HPP

#include <QWidget>
#include "adwidgets_global.hpp"

class QGridLayout;
class QLabel;

namespace adwidgets {

    class ADWIDGETSSHARED_EXPORT PressureGauge : public QWidget {
        Q_OBJECT
    public:
        ~PressureGauge();
        PressureGauge( QWidget *parent = 0 );
        PressureGauge( Qt::Orientation, QWidget *parent = 0 );
        PressureGauge( Qt::Orientation, QGridLayout *, int row, int col, QWidget *parent = 0);

        void setTitle( const QString& );
        void setActualValue( double value );
        double value() const;

    private:
        class impl;
        impl * impl_;

    signals:

    public slots:

    };

}

#endif // PRESSUREGUAGE_HPP
