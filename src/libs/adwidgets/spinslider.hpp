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

#ifndef SPINSLIDER_HPP
#define SPINSLIDER_HPP

#include <QWidget>
#include "adwidgets_global.hpp"

class QGridLayout;
class QLabel;

namespace adwidgets {

    class ADWIDGETSSHARED_EXPORT SpinSlider : public QWidget {
        Q_OBJECT

        SpinSlider( const SpinSlider& ) = delete;
        SpinSlider& operator = ( const SpinSlider& ) = delete;

    public:
        ~SpinSlider();
        SpinSlider(QWidget *parent = 0);
        SpinSlider(Qt::Orientation, QWidget *parent = 0);
        SpinSlider(Qt::Orientation, QGridLayout *, int row, int col, QWidget *parent = 0);

        void setTitle( const QString& );
        void setRange( const QPair< double, double >& );
        void setActualValue( double value );
        void setValue( double );
        double value() const;
        QLabel * labelActual();
        QLabel * labelHeader();

    private:
        class impl;
        impl * impl_;

    signals:
        void valueChanged( double );

    public slots:

    private slots:
        void handleSpinValueChanged( double );
        void handleSliderValueChanged( int );
    };

}

#endif // SPINSLIDER_HPP
